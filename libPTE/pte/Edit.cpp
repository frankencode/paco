#include <QDebug> // DEBUG
// #include <ftl/PrintDebug.hpp> // DEBUG
#include <QTimer>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QShortcut>
#include <QLabel>
#include <QStringListModel>
#include <codetips/codetips.hpp>
#include <codetips/Assistant.hpp>
#include "Context.hpp"
#include "MembersTipModel.hpp"
#include "QuickHelp.hpp"
#include "TextElider.hpp"
#include "SearchPopup.hpp"
#include "Edit.hpp"

#if PTE_IM_WORKAROUND
#warning "Using workaround for broken input method code."
#endif

namespace pte
{

Edit::Edit(QWidget* parent)
	: View(parent, defaultMetrics()),
	  cy_(-1), cx_(-1),
	  cy2_(0), cx2_(0),
	  cxSaved_(-1),
	  needFullUpdate_(false),
	  hasBracketMatch_(false),
	  bmy0_(-1), bmx0_(-1),
	  bmy1_(-1), bmx1_(-1),
	  autoIndent_(true),
	  indent_("\t"),
	  preeditCursor_(0),
	  #if PTE_IM_WORKAROUND
	  ignoreNextKeyPressEvent_(false),
	  #endif
	  assistantEdit_(false),
	  assistantModify_(false),
	  eatenTabBefore_(false)
{
	setLeadingMargin(2);
	setAttribute(Qt::WA_InputMethodEnabled, true);
}

Edit::~Edit() {}

Ref<ViewMetrics, Owner> Edit::defaultMetrics()
{
	Ref<ViewMetrics, Owner> metrics = View::defaultMetrics();
	metrics->cursorStyle_ = BeamCursor|BlinkingCursor;
	return metrics;
}

bool Edit::autoIndent() const { return autoIndent_; }
void Edit::setAutoIndent(bool on) { autoIndent_ = on; }

QString Edit::indent() const { return indent_; }
void Edit::setIndent(QString indent) { indent_ = indent; }

QSize Edit::sizeHint() const { return QSize(200, 100); }

int Edit::cursorLine() const { return cy_; }
int Edit::cursorColumn() const { return cx_ + preeditCursor_; }

bool Edit::moveCursor(int cy, int cx)
{
	if (cy >= cache()->length()) cy = cache()->length() - 1;
	if (cy < 0) cy = 0;
	int cySaved = cy_;
	int cxSaved = cx_;
	cy_ = cy;
	cx_ = cx;
	if ((cy_ != cySaved) || (cx_ != cxSaved)) {
		emit cursorMoved(cy, cx);
		matchBrackets();
	}
	return true;
}

void Edit::insert(const QString& text)
{
	paste(text, false);
}

void Edit::move(int deltaChars)
{
	highlighterYield();
	int cySaved = cy_;
	int cxSaved = cx_;
	stepCursor(&cy_, &cx_, deltaChars);
	bool moved = ((cy_ != cySaved) || (cx_ != cxSaved));
	if (moved) matchBrackets();
	highlighterResume();
	if (moved) emit cursorMoved(cy_, cx_);
}

void Edit::select(int deltaChars)
{
	highlighterYield();
	int cy2 = cy_;
	int cx2 = cx_;
	stepCursor(&cy2, &cx2, deltaChars);
	View::select(cy_, cx_, cy2, cx2);
	highlighterResume();
	update();
}

QString Edit::copy(int deltaChars)
{
	QString text;
	highlighterYield();
	int cy2 = cy_;
	int cx2 = cx_;
	stepCursor(&cy2, &cx2, deltaChars);
	text = document()->copy(cy_, cx_, cy2, cx2);
	highlighterResume();
	return text;
}

void Edit::undo() { beginEdit(); undoRedo(Document::Undo); endEdit(); }
void Edit::undoForget() { beginEdit(); undoRedo(Document::Undo, true); endEdit(); }
void Edit::redo() { beginEdit(); undoRedo(Document::Redo); endEdit(); }
void Edit::copy() { highlighterYield(); copyToClipboard(); highlighterResume(); }
void Edit::cut() { beginEdit(); cutSelection(); endEdit(); }
void Edit::paste() { beginEdit(); pasteAtCursor(); endEdit(); }
void Edit::paste(QString text, bool moveCursor) { beginEdit(); pasteAtCursor(text, moveCursor); endEdit(); }

void Edit::selectWord()
{
	int selx0, selx1;
	highlighterYield();
	if (expandToWord(cy_, cx_, &selx0, &selx1)) {
		if (!hasSelection())
			updateAfterCursorMoved(cy_, cy_);
		else
			update();
		View::select(cy_, selx0, cy_, selx1);
		// update();
	}
	else if (hasSelection()) {
		unselect();
		update();
	}
	highlighterResume();
}

void Edit::selectLine()
{
	highlighterYield();
	View::select(cy_, 0, cy_, intMax);
	cx_ = 0;
	matchBrackets();
	blinkRestart();
	highlighterResume();
	update();
}

void Edit::selectAll()
{
	highlighterYield();
	View::selectAll();
	highlighterResume();
	update();
}

void Edit::duplicateLine()
{
	beginEdit();
	beginModify();
	if (hasSelection()) unselect();
	QString ln = document()->copySpan(cy_, 0, intMax) + "\n";
	document()->push(cy_, 0, 0, 0, ln);
	endModify();
	endEdit();
}

void Edit::stepCursor(int* cy, int* cx, int deltaChars)
{
	while (deltaChars > 0) {
		document()->stepForward(cy, cx);
		--deltaChars;
	}
	while (deltaChars < 0) {
		document()->stepBackward(cy, cx);
		++deltaChars;
	}
}

void Edit::undoRedo(int dir, bool forget)
{
	if (document()->hasHistory(dir)) {
		if (hasSelection())
			unselect();
		restoreState(document()->stepHistory(dir, forget));
		needFullUpdate_ = true; // quick HACK, more fine-grained control could benefit performance
	}
}

void Edit::copyToClipboard()
{
	if (hasSelection()) {
		int selya, selxa, selyb, selxb;
		getSelection(&selya, &selxa, &selyb, &selxb);
		QString s = document()->copy(selya, selxa, selyb, selxb);
		QApplication::clipboard()->setText(s);
	}
}

void Edit::deleteSelection()
{
	View::deleteSelection();
}

void Edit::cutSelection()
{
	copyToClipboard();
	beginModify();
	deleteSelection();
	endModify();
}

void Edit::pasteAtCursor()
{
	pasteAtCursor(QApplication::clipboard()->text());
}

void Edit::pasteAtCursor(QString text, bool moveCursor)
{
#ifdef Q_WS_MAC
	// old-style Apple newlines, HACK
	if (text.contains(13) && (!text.contains(10)))
		text.replace(13, 10);
#endif
	if ((cy_ != -1) && (cx_ != -1) && (text.length() > 0))
	{
		beginModify();
		deleteSelection();
		if (moveCursor)
			document()->push(cy_, cx_, &cy_, &cx_, text);
		else
			document()->push(cy_, cx_, 0, 0, text);
		endModify();
	}
}

bool Edit::hasBracketMatch() const { return hasBracketMatch_; }

void Edit::getBracketMatch(int* bmy0, int* bmx0, int* bmy1, int* bmx1)
{
	*bmy0 = bmy0_;
	*bmx0 = bmx0_;
	*bmy1 = bmy1_;
	*bmx1 = bmx1_;
}

void Edit::matchBrackets()
{
	bool hadBracketMatch = hasBracketMatch_;
	int bmy0Saved = bmy0_, bmx0Saved = bmx0_;
	int bmy1Saved = bmy1_, bmx1Saved = bmx1_;
	
	QString s0 = document()->copySpan(cy_, cx_ - 1, cx_);
	QString s1 = document()->copySpan(cy_, cx_, cx_ + 1);
	
	if ((s0.length() > 0) && QString(")}]").contains(s0))
		matchBrackets(cy_, cx_ - 1);
	else
		matchBrackets(cy_, cx_);
	
	if ((!hasBracketMatch()) && (cx_ > 0))
		matchBrackets(cy_, cx_ - 1);
	
	needFullUpdate_ = needFullUpdate_ ||
		( (hadBracketMatch != hasBracketMatch_) ||
		  ( (hasBracketMatch_ == true) &&
		    ( (bmy0Saved != bmy0_) ||
		      (bmx0Saved != bmx0_) ||
		      (bmy1Saved != bmy1_) ||
		      (bmx1Saved != bmx1_) ) ) );
}

void Edit::matchBrackets(int cy, int cx)
{
	hasBracketMatch_ = false;
	bmy0_ = -1; 
	bmx0_ = -1;
	bmy1_ = -1;
	bmx1_ = -1;
	
	QString s;
	Token defaultToken;
	Ref<Token> token;
	document()->getCell(cy, cx, &s, 0, &token);
	if (s.isEmpty()) return;
	
	QChar ch = s[0];
	QChar ob, cb; // opening and closing bracket
	bool forward; // search direction
	
	if ((ch == '(') || (ch == ')')) {
		ob = '(';
		cb = ')';
		forward = (ch == '(');
	}
	else if ((ch == '[') || (ch == ']')) {
		ob = '[';
		cb = ']';
		forward = (ch == '[');
	}
	else if ((ch == '{') || (ch == '}')) {
		ob = '{';
		cb = '}';
		forward = (ch == '{');
	}
	else
		return;
	
	{
		int n = 0x10000; // security HACK, search limit
		int y = cy, x = cx;
		int depth = 0;
		
		while ((!hasBracketMatch_) && ((0 <= y) && (y < cache()->length())) && (n > 0))
		{
			QString ln = document()->copySpan(y, 0, intMax, ' ', true);
			
			if (forward) {
				while (x < ln.length()) {
					if (ln.at(x) == ob) {
						QString s2;
						Ref<Token> token2;
						document()->getCell(y, x, &s2, 0, &token2);
						if ((token == token2) || (!(token2)) || (!(token))) {
							bool goAhead = true;
							if ((!(token)) && (token2)) goAhead = (token2->length() == 1);
							else if ((!(token2)) && (token)) goAhead = (token->length() == 1);
							depth += goAhead;
						}
						else if ((token) && (token2)) {
							if ( (token->definition() == token2->definition()) &&
							     (token->rule() == token2->rule()) )
								++depth;
						}
					}
					else if (ln.at(x) == cb) {
						QString s2;
						Ref<Token> token2;
						document()->getCell(y, x, &s2, 0, &token2);
						if ((token == token2) || (!(token2)) || (!(token))) {
							bool goAhead = true;
							if ((!(token)) && (token2)) goAhead = (token2->length() == 1);
							else if ((!(token2)) && (token)) goAhead = (token->length() == 1);
							depth -= goAhead;
						}
						else if ((token) && (token2)) {
							if ( (token->definition() == token2->definition()) &&
							     (token->rule() == token2->rule()) )
								--depth;
						}
						if (depth == 0) {
							hasBracketMatch_ = true;
							bmy0_ = cy;
							bmx0_ = cx;
							bmy1_ = y;
							bmx1_ = x;
							break;
						}
					}
					++x;
				}
				
				++y;
				x = 0;
			}
			else {
				if (x == intMax)
					x = ln.length() - 1;
				while (x >= 0) {
					if (ln.at(x) == ob) {
						QString s2;
						Ref<Token> token2;
						document()->getCell(y, x, &s2, 0, &token2);
						if ((token == token2) || (!(token2)) || (!(token))) {
							bool goAhead = true;
							if ((!(token)) && (token2)) goAhead = (token2->length() == 1);
							else if ((!(token2)) && (token)) goAhead = (token->length() == 1);
							depth -= goAhead;
						}
						else if ((token) && (token2)) {
							if ( (token->definition() == token2->definition()) &&
							     (token->rule() == token2->rule()) )
								--depth;
						}
						if (depth == 0) {
							hasBracketMatch_ = true;
							bmy0_ = cy;
							bmx0_ = cx;
							bmy1_ = y;
							bmx1_ = x;
							break;
						}
					}
					else if (ln.at(x) == cb) {
						QString s2;
						Ref<Token> token2;
						document()->getCell(y, x, &s2, 0, &token2);
						if ((token == token2) || (!(token2)) || (!(token))) {
							bool goAhead = true;
							if ((!(token)) && (token2)) goAhead = (token2->length() == 1);
							else if ((!(token2)) && (token)) goAhead = (token->length() == 1);
							depth += goAhead;
						}
						else if ((token) && (token2)) {
							if ( (token->definition() == token2->definition()) &&
							     (token->rule() == token2->rule()) )
								++depth;
						}
					}
					--x;
				}
				
				--y;
				x = intMax;
			}
			--n;
		}
	}
}

void Edit::beginEdit()
{
	if (assistantEdit_) return;
	
	highlighterYield();
	
	cy2_ = cy_;
	cx2_ = cx_;
	getViewRange(&vya2_, &vxa2_, &vyb2_, &vxb2_, &tinyDoc2_);
	needFullUpdate_ = false;
}

void Edit::endEdit()
{
	if (assistantEdit_) return;
	
	updateVerticalScrollRange();
	keepInView();
	if ((cy_ != cy2_) || (cx_ != cx2_))
		emit cursorMoved(cy_, cx_);
	matchBrackets();
	blinkRestart();
	
	int y0 = 0, y1 = 0;
	int deltaType = document()->classifyLastDelta(&y0, &y1);
	if ( (y0 == y1) &&
		 ((deltaType == Delta::PushSpan) || (deltaType == Delta::PopSpan)) &&
		 (!needFullUpdate_) )
		updateAfterLineModified(y0 /*cy_*/);
	else
		update();
	
	highlighterRestart();
	highlighterResume();
}

void Edit::beginModify()
{
	if (assistantModify_) return;
	document()->begin(saveState());
}

void Edit::endModify()
{
	if (assistantModify_) return;
	document()->end(saveState());
}

void Edit::assistantBeginEdit()
{
	if (!assistantEdit_) {
		beginEdit();
		assistantEdit_ = true;
	}
}

void Edit::assistantBeginModify()
{
	if (!assistantModify_) {
		beginModify();
		assistantModify_ = true;
	}
}

void Edit::assistantEnd()
{
	if (assistantModify_) {
		assistantModify_ = false;
		endModify();
	}
	if (assistantEdit_) {
		assistantEdit_ = false;
		endEdit();
	}
}

bool Edit::isWord(QChar ch) const
{
	return (('a' <= ch) && (ch <= 'z')) ||
	       (('A' <= ch) && (ch <= 'Z')) ||
	       (('0' <= ch) && (ch <= '9')) ||
	       (ch == '_');
}

void Edit::stepPreviousWord(int* cy, int* cx)
{
	QString ch;
	document()->getCell(*cy, *cx - 1, &ch);
	if (ch == QString()) return;
	
	bool invert = isWord(ch.at(0));
	--*cx;
	while (*cx > 0) {
		document()->getCell(*cy, *cx - 1, &ch);
		if (ch == QString()) break;
		bool done = isWord(ch.at(0));
		if (invert) done = !done;
		if (done) break;
		--*cx;
	}
}

void Edit::stepNextWord(int* cy, int* cx)
{
	QString ch;
	document()->getCell(*cy, *cx, &ch);
	if (ch == QString()) return;
	
	bool invert = isWord(ch.at(0));
	while (true) {
		++*cx;
		document()->getCell(*cy, *cx, &ch);
		if (ch == QString()) break;
		bool done = isWord(ch.at(0));
		if (invert) done = !done;
		if (done) break;
	}
}

bool Edit::getCodetips(QKeyEvent* event)
{
	using namespace codetips;
	
	hideTip();
	
	int modifiers =
		((event->modifiers() & Qt::ShiftModifier)   != 0) * Assistant::Shift   |
		((event->modifiers() & Qt::AltModifier)     != 0) * Assistant::Alt     |
		((event->modifiers() & Qt::ControlModifier) != 0) * Assistant::Control |
		((event->modifiers() & Qt::MetaModifier)    != 0) * Assistant::Meta;
	
	uchar_t key = 0;
	
	if (event->key() == Qt::Key_Backtab) {
		modifiers |= Assistant::Shift;
		key = '\t';
	}
	else if (event->key() == Qt::Key_Tab) {
		key = '\t';
	}
	else if (event->key() == Qt::Key_Return) {
		key = '\n';
	}
	else {
		QString text = event->text();
		if (text.length() > 0) key = text.at(0).unicode();
	}
	
	Ref<Tip, Owner> tip;
	
	if (key > 0) {
		tip = AssistantManager::instance()->assist(context(), modifiers, key);
		
		if (tip) {
			if (tip->typeId() == TypeTip::Id) {
				Ref<TypeTip> typeTip = tip;
				new QuickHelp(this, cursorRenderPos(), typeTip->type()->displayString()->data());
			}
			else if (tip->typeId() == MembersTip::Id) {
				Ref<MembersTip> membersTip = tip;
				Ref<Members> members = membersTip->members();
				bool noTypes = true;
				for (int i = 0; members->has(i) && noTypes; ++i)
					if (members->at(i)->type())
						noTypes = false;
				MembersTipModel* model = new MembersTipModel(members);
				QLabel* quickInfo = 0;
				if (!noTypes) {
					quickInfo = new QLabel;
					quickInfo->setStyleSheet(
						QString(
							"QLabel {"
							"  font-size: %1pt;"
							"}"
						).arg(qApp->font().pointSize() - 2)
					);
				}
				QString word;
				{
					highlighterYield();
					int wx0 = -1, wx1 = -1;
					if ((!expandToWord(cy_, cx_, &wx0, &wx1)) && (cx_ > 0))
						expandToWord(cy_, cx_ - 1, &wx0, &wx1);
					if ((wx0 != -1) && (wx1 != -1)) {
						beginModify();
						document()->popSpan(cy_, wx0, wx1, &word);
						cx_ = wx0;
						endModify();
					}
					highlighterRestart();
					highlighterResume();
				}
				renderTo();
				searchPopup_ = new SearchPopup(this, cursorRenderPos(), model, quickInfo, word);
				connect(searchPopup_, SIGNAL(rejected()), this, SLOT(undoForget()));
				if (!noTypes) {
					connect(searchPopup_, SIGNAL(currentChanged(const QModelIndex&)), model, SLOT(currentChanged(const QModelIndex&)));
					TextElider* elider = new TextElider(quickInfo, Qt::ElideMiddle);
					connect(model, SIGNAL(memberInfo(const QString&)), elider, SLOT(setText(const QString&)));
					connect(elider, SIGNAL(textChanged(const QString&)), quickInfo, SLOT(setText(const QString&)));
				}
			}
			else if (tip->typeId() == WordsTip::Id) {
				QString word;
				{
					highlighterYield();
					int wx0 = -1, wx1 = -1;
					if ((!expandToWord(cy_, cx_, &wx0, &wx1)) && (cx_ > 0))
						expandToWord(cy_, cx_ - 1, &wx0, &wx1);
					if ((wx0 != -1) && (wx1 != -1)) {
						beginModify();
						document()->popSpan(cy_, wx0, wx1, &word);
						cx_ = wx0;
						endModify();
					}
					highlighterRestart();
					highlighterResume();
				}
				Ref<WordsTip> wordsTip = tip;
				Ref<StringList> words = wordsTip->words();
				QStringList qWords;
				for (int i = 0; i < words->length(); ++i)
					qWords << QString::fromUtf8(words->at(i));
				QStringListModel* model = new QStringListModel(qWords);
				renderTo();
				searchPopup_ = new SearchPopup(this, cursorRenderPos(), model, 0, word);
				connect(searchPopup_, SIGNAL(rejected()), this, SLOT(undoForget()));
			}
		}
	}
	
	if (!((key == '\t') && (modifiers == 0)))
		eatenTabBefore_ = false;
	
	if (!eatenTabBefore_) {
		if ((!tip) && (key == '\t') && (modifiers == 0)) {
			if (context()->indentOf(context()->line()).length() < context()->linePos()) {
				tip = new Tip;
				eatenTabBefore_ = true;
			}
		}
	}
	
	bool ret = assistantModify_ || (tip);
	assistantEnd();
	return ret;
}

void Edit::keyPressEvent(QKeyEvent* event)
{
	int key = event->key();
	int modifiers = event->modifiers();
	bool shift = ((modifiers & Qt::ShiftModifier) != 0);
	bool alt = ((modifiers & Qt::AltModifier) != 0);
	bool control = ((modifiers & Qt::ControlModifier) != 0);
	bool meta = ((modifiers & Qt::MetaModifier) != 0);
	
	// ignore modifier key press events
	if ( (key == Qt::Key_Shift) ||
	     (key == Qt::Key_Control) ||
	     (key == Qt::Key_Meta) ||
	     (key == Qt::Key_Alt) ||
	     (key == Qt::Key_AltGr) ||
	     (key == Qt::Key_CapsLock) ) {
		QWidget::keyPressEvent(event);
		return;
	}
	
	if (getCodetips(event)) return;
	
	// -----------------------------------------------------------
	// navigation keys
	
	if ( (key == Qt::Key_Up) ||
	     (key == Qt::Key_Down) ||
	     (key == Qt::Key_Left) ||
	     (key == Qt::Key_Right) ||
	     (key == Qt::Key_Home) ||
	     (key == Qt::Key_End) ||
	     (key == Qt::Key_PageUp) ||
	     (key == Qt::Key_PageDown) ||
	     (meta && (key == Qt::Key_P)) ||
	     (meta && (key == Qt::Key_N)) ||
	     (meta && (key == Qt::Key_B)) ||
	     (meta && (key == Qt::Key_F)) ||
	     (meta && (key == Qt::Key_A)) ||
	     (meta && (key == Qt::Key_E)) ||
	     (meta && (key == Qt::Key_V)) )
	{
		highlighterYield();
		
		int cy2 = cy_;
		int cx2 = cx_;
	
		if ( (key == Qt::Key_Up) ||
		     (key == Qt::Key_Down) ||
		     (key == Qt::Key_PageUp) ||
		     (key == Qt::Key_PageDown) )
		{
			if (cxSaved_ == -1)
				cxSaved_ = cx_;
		}
		else {
			cxSaved_ = -1;
		}
		
		if (shift && (!hasSelection()))
			startSelection(cy_, cx_);
		
		bool wordwise = alt;
		#ifdef Q_WS_X11
		wordwise = wordwise || control;
		#endif
		wordwise = wordwise && ((!hasSelection()) || shift);
		
		bool longJump = meta||control;
		
		if (((key == Qt::Key_Left) && (!longJump)) || (meta && (key == Qt::Key_B))) {
			if (wordwise)
				stepPreviousWord(&cy_, &cx_);
			else
				document()->stepBackward(&cy_, &cx_);
		}
		else if (((key == Qt::Key_Right) && (!longJump)) || (meta && (key == Qt::Key_F))) {
			if (wordwise)
				stepNextWord(&cy_, &cx_);
			else
				document()->stepForward(&cy_, &cx_);
		}
		else if ((key == Qt::Key_Up) || (meta && (key == Qt::Key_P))) {
			--cy_;
			cx_ = cxSaved_;
			document()->stepNearest(&cy_, &cx_);
		}
		else if ((key == Qt::Key_Down) || (meta && (key == Qt::Key_N))) {
			++cy_;
			cx_ = cxSaved_;
			document()->stepNearest(&cy_, &cx_);
		}
		else if ((key == Qt::Key_Home) || ((key == Qt::Key_Left) && longJump) || (meta && (key == Qt::Key_A))) {
			QString ln = document()->copySpan(cy_, 0, intMax, ' ', true);
			int cx0 = 0;
			while (cx0 < ln.length()) {
				if (!ln.at(cx0).isSpace()) break;
				++cx0;
			}
			if ((cx_ != cx0) && (cx_ != 0))
				cx_ = cx0;
			else
				cx_ = 0;
		}
		else if ((key == Qt::Key_End) || ((key == Qt::Key_Right) && longJump) || (meta && (key == Qt::Key_E))) {
			cx_ = 0;
			Ref<ChunkList> chunkList = cache()->get(cy_)->chunkList_;
			if (chunkList->length() > 0)
				cx_ = chunkList->get(-1)->x1_;
		}
		else if (key == Qt::Key_PageUp) {
			cy_ -= windowLines() / 2;
			if (cy_ < 0) cy_ = 0;
			cx_ = cxSaved_;
			document()->stepNearest(&cy_, &cx_);
		}
		else if ((key == Qt::Key_PageDown) || (meta && (key == Qt::Key_V))) {
			cy_ += windowLines() / 2;
			if (cy_ >= cache()->length()) cy_ = cache()->length() - 1;
			cx_ = cxSaved_;
			document()->stepNearest(&cy_, &cx_);
		}
		
		needFullUpdate_ = false;
		
		if (shift) {
			extendSelection(cy_, cx_);
		}
		else if (hasSelection() && (!alt)) {
			unselect();
			needFullUpdate_ = true;
		}
		
		keepInView();
		matchBrackets();
		
		if ( ((cy_ != cy2) || (cx_ != cx2)) &&
		     (!needFullUpdate_) ) {
			emit cursorMoved(cy_, cx_);
			if (!needFullUpdate_)
				updateAfterCursorMoved(cy_, cy2);
		}
		else
			needFullUpdate_ = true;
		if (needFullUpdate_)
			update();
		blinkRestart();
		
		highlighterResume();
	}
	else if (key == Qt::Key_Escape)
	{
		highlighterYield();
		
		if (hasSelection()) {
			unselect();
			setShowFindResult(false);
		}
		else {
			setShowFindResult(!showFindResult());
		}
		
		highlighterResume();
	}
	
	// -----------------------------------------------------------
	// modifying keys
	
	else if (key == Qt::Key_Return) {
		beginEdit();
		beginModify();
		deleteSelection();
		QString s = "\n";
		QString ln = document()->copySpan(cy_, 0, intMax);
		if ((autoIndent_) && (cy_ > 0)) {
			int wc = 0;
			while ((wc < ln.length()) && (ln.at(wc) <= 0x20)) ++wc;
			if (wc > 0) {
				if (cx_ > 0)
					s = s + ln.left(wc);
				else
					s = ln.left(wc) + s;
			}
		}
		/*
		QChar ch = '\0';
		if ((ln.length() > 0) && (cx_ > 0)) {
			ch = ln.at((cx_ >= ln.length()) ? ln.length() - 1 : cx_ - 1);
			if ((ch == ':') || (ch == '(') || (ch == '{') || (ch == '>'))
				s = s + indent_;
		}*/
		document()->push(cy_, cx_, &cy_, &cx_, s);
		/*if (ch == '{') {
			if (document()->copySpan(cy_, cx_, cx_ + 1) == "}")
				document()->push(cy_, cx_, 0, 0, "\n");
		}*/
		endModify();
		endEdit();
	}
	else if ((key == Qt::Key_Tab) || (key == Qt::Key_Backtab)) {
		int selya, selxa, selyb, selxb;
		getSelection(&selya, &selxa, &selyb, &selxb);
		if (hasSelection() && (selya < selyb)) {
			beginEdit();
			beginModify();
			if ((selxa > 0) || ((0 < selxb) && (selxb < intMax))) {
				selxa = 0;
				if ((0 < selxb) && (selxb != intMax)) selxb = intMax;
				cx_ = 0;
				if (cy_ == selyb)
					cy_ += (selxb == intMax);
				View::select(selya, selxa, selyb, selxb);
			}
			selyb += (selxb > 0);
			for (int y = selya; y < selyb; ++y) {
				int tc = indent_.count('\t');
				if ((tc == 0) && (document()->copySpan(y, 0, intMax).trimmed().isEmpty())) continue;
				if (key == Qt::Key_Backtab) {
					int indentWidth = (indent_.length() - tc) + tc * document()->tabWidth();
					if (document()->copySpan(y, 0, indentWidth) == indent_)
						document()->popSpan(y, 0, indentWidth);
					else if (document()->copySpan(y, 0, 1) == " ")
						document()->popSpan(y, 0, 1);
				}
				else {
					int h;
					document()->pushSpan(y, 0, &h, indent_);
				}
			}
			endModify();
			endEdit();
		}
		else {
			if (key == Qt::Key_Tab) {
				beginEdit();
				beginModify();
				if (hasSelection())
					deleteSelection();
				int len = indent_.length();
				if (len > 1) {
					int dx = 1;
					while ((cx_ + dx) % len != 0) ++dx;
					len = dx;
				}
				document()->push(cy_, cx_, &cy_, &cx_, indent_.left(len));
				endModify();
				endEdit();
			}
			else {
				QString prefix = document()->copySpan(cy_, 0, cx_);
				if ((prefix.length() > 0) && (prefix.trimmed().length() == 0) && (!hasSelection())) {
					if (prefix.at(prefix.length() - 1) == ' ') {
						beginEdit();
						beginModify();
						int indentWidth = (indent_ == "\t") ? metrics()->tabWidth_ : indent_.length();
						int cy2 = cy_;
						int cx2 = cx_;
						int cxe = cx2 - 1;
						while (cxe % indentWidth != 0) --cxe;
						if (cxe < 0) cxe = 0;
						while (cx2 > cxe)
							document()->stepBackward(&cy2, &cx2);
						document()->pop(cy2, cx2, cy_, cx_);
						cy_ = cy2;
						cx_ = cx2;
						endModify();
						endEdit();
					}
					else {
						beginEdit();
						beginModify();
						int cy2 = cy_;
						int cx2 = cx_;
						document()->stepBackward(&cy2, &cx2);
						document()->pop(cy2, cx2, cy_, cx_);
						cy_ = cy2;
						cx_ = cx2;
						endModify();
						endEdit();
					}
				}
			}
		}
	}
	else if ( ((key == Qt::Key_Backspace) && ((!document()->beginOfDocument(cy_, cx_)) || hasSelection())) ||
	          ((key == Qt::Key_Delete) && ((!document()->endOfDocument(cy_, cx_)) || hasSelection())) )
	{
		beginEdit();
		int selya, selxa, selyb, selxb;
		getSelection(&selya, &selxa, &selyb, &selxb);
		bool blockOperation = hasSelection() && (selya < selyb - 1) && alt;
		if ((key == Qt::Key_Backspace) && (cx_ == 0))
			blockOperation = false;
		if (hasSelection() && (!blockOperation)) {
			beginModify();
			deleteSelection();
			endModify();
		}
		else {
			int cya = selya;
			int cyb = selyb + ((0 < selxb) && (selxb < intMax));
			if (!blockOperation) {
				cya = cy_;
				cyb = cya + 1;
			}
			int cySaved = cy_;
			int cxSaved = cx_;
			beginModify();
			for (int cy = cya; cy < cyb; ++cy) {
				cy_ = cy;
				cx_ = cxSaved;
				QString ln = document()->copySpan(cy, 0, intMax, ' ', true);
				if (ln.length() >= cx_ - (key == Qt::Key_Delete)) {
					int cy2 = cy_;
					int cx2 = cx_;
					if (key == Qt::Key_Backspace)
						document()->stepBackward(&cy2, &cx2);
					else
						document()->stepForward(&cy2, &cx2);
					if ((cy_ != cy2) || (cx_ != cx2)) {
						if (key == Qt::Key_Backspace) {
							document()->pop(cy2, cx2, cy_, cx_);
							cy_ = cy2;
							cx_ = cx2;
						}
						else
							document()->pop(cy_, cx_, cy2, cx2);
					}
				}
			}
			if (blockOperation)
				cy_ = cySaved;
			endModify();
		}
		endEdit();
	}
	else if ((event->text().length() > 0) && (!(control || /*alt ||*/ meta)))
	{
		QString text = event->text();
		bool printable = true;
		#if PTE_IM_WORKAROUND
		if (ignoreNextKeyPressEvent_) {
			ignoreNextKeyPressEvent_ = false;
			printable = false;
		}
		#endif
		for (int i = 0; (i < text.size()) && printable; ++i)
			printable = printable && text.at(i).isPrint();
		if (printable) {
			beginEdit();
			beginModify();
			int selya, selxa, selyb, selxb;
			getSelection(&selya, &selxa, &selyb, &selxb);
			bool blockOperation = (selya < selyb - 1);
			if (hasSelection() && blockOperation) {
				selyb += (0 < selxb) && (selxb < intMax);
				int cx2 = cx_;
				for (int y = selya; y < selyb; ++y) {
					QString ln = document()->copySpan(y, 0, intMax, ' ', true);
					if (ln.length() >= cx_)
						document()->pushSpan(y, cx_, &cx2, text);
				}
				cx_ = cx2;
			}
			else {
				deleteSelection();
				document()->push(cy_, cx_, &cy_, &cx_, text);
			}
			endModify();
			endEdit();
		}
	}
	
	// QWidget::keyPressEvent(event);
}

void Edit::inputMethodEvent(QInputMethodEvent* event)
{
	bool beginInput = (event->preeditString() != preeditString_);
	bool finishInput = (!event->commitString().isEmpty()) || (event->replacementLength() > 0);
	bool modifying = (beginInput && hasSelection()) || finishInput;
	
	if (!preeditString_.isEmpty()) {
		highlighterYield();
		document()->setHistoryEnabled(false);
		document()->setEmissionEnabled(false);
		document()->popSpan(cy_, cx_, cx_ + preeditString_.length());
		document()->setHistoryEnabled(true);
		document()->setEmissionEnabled(true);
		highlighterRestart();
		highlighterResume();
	}
	
	if (modifying) {
		beginEdit();
		beginModify();
		deleteSelection();
	}
	else {
		highlighterYield();
	}
	
	for (int i = 0, n = event->attributes().size(); i < n; ++i)
	{
		const QInputMethodEvent::Attribute& a = event->attributes().at(i);
		if (a.type == QInputMethodEvent::Selection)
			View::select(cy_, cx_ + a.start, cy_, cx_ + a.start + a.length);
		else if (a.type == QInputMethodEvent::Cursor)
			preeditCursor_ = a.start * (a.length > 0); // quick HACK, unclear semantics
	}
	
	if (finishInput) {
		cx_ += event->replacementStart();
		if (event->replacementLength() > 0)
			document()->popSpan(cy_, cx_, cx_ + event->replacementLength());
		if (!event->commitString().isEmpty())
			document()->pushSpan(cy_, cx_, &cx_, event->commitString());
		preeditString_ = QString();
		preeditCursor_ = 0;
		#if PTE_IM_WORKAROUND
		ignoreNextKeyPressEvent_ = !event->commitString().isEmpty();
		#endif
	}
	else {
		preeditString_ = event->preeditString();
		if (!preeditString_.isEmpty()) {
			document()->setHistoryEnabled(false);
			document()->setEmissionEnabled(false);
			// preeditStyle_ = new Style(colors()->foregroundColor_, colors()->matchColor_);
			document()->pushSpan(cy_, cx_, 0, preeditString_/*, preeditStyle_*/);
			document()->setHistoryEnabled(true);
			document()->setEmissionEnabled(true);
		}
		update();
	}
	
	if (modifying) {
		endModify();
		endEdit();
	}
	else {
		highlighterRestart();
		highlighterResume();
	}
	
	event->accept();
	blinkRestart();
}

QMenu* Edit::setupActions(QWidget* parent)
{
	Edit* edit = qobject_cast<Edit*>(parent);
	
	QAction* undo = new QAction(tr("Undo"), parent);
	QAction* redo = new QAction(tr("Redo"), parent);
	QAction* copy = new QAction(tr("Copy"), parent);
	QAction* cut = new QAction(tr("Cut"), parent);
	QAction* paste = new QAction(tr("Paste"), parent);
	QAction* selectWord = new QAction(tr("Select Word"), parent);
	QAction* selectLine = new QAction(tr("Select Line"), parent);
	QAction* selectAll = new QAction(tr("Select All"), parent);
	QAction* duplicateLine = new QAction(tr("Duplicate Line"), parent);
	
	undo->setShortcut(tr("Ctrl+Z"));
	redo->setShortcut(tr("Ctrl+Shift+Z"));
	copy->setShortcut(tr("Ctrl+C"));
	cut->setShortcut(tr("Ctrl+X"));
	paste->setShortcut(tr("Ctrl+V"));
	selectWord->setShortcut(tr("Ctrl+K"));
	selectLine->setShortcut(tr("Ctrl+L"));
	selectAll->setShortcut(tr("Ctrl+A"));
	duplicateLine->setShortcut(tr("Ctrl+D"));
	
	if (edit) {
		edit->addAction(undo);
		edit->addAction(redo);
		edit->addAction(copy);
		edit->addAction(cut);
		edit->addAction(paste);
		edit->addAction(selectWord);
		edit->addAction(selectLine);
		edit->addAction(selectAll);
		edit->addAction(duplicateLine);
		
		connect(undo, SIGNAL(triggered()), edit, SLOT(undo()));
		connect(redo, SIGNAL(triggered()), edit, SLOT(redo()));
		connect(copy, SIGNAL(triggered()), edit, SLOT(copy()));
		connect(cut, SIGNAL(triggered()), edit, SLOT(cut()));
		connect(paste, SIGNAL(triggered()), edit, SLOT(paste()));
		connect(selectWord, SIGNAL(triggered()), edit, SLOT(selectWord()));
		connect(selectLine, SIGNAL(triggered()), edit, SLOT(selectLine()));
		connect(selectAll, SIGNAL(triggered()), edit, SLOT(selectAll()));
		connect(duplicateLine, SIGNAL(triggered()), edit, SLOT(duplicateLine()));
		
		QShortcut* altCopy = new QShortcut(tr("Ctrl+INSERT"), edit);
		QShortcut* altCut = new QShortcut(tr("Shift+DELETE"), edit);
		QShortcut* altPaste = new QShortcut(tr("Shift+INSERT"), edit);
		altCopy->setContext(Qt::WidgetShortcut);
		altCut->setContext(Qt::WidgetShortcut);
		altPaste->setContext(Qt::WidgetShortcut);
		connect(altCopy, SIGNAL(activated()), edit, SLOT(copy()));
		connect(altCut, SIGNAL(activated()), edit, SLOT(cut()));
		connect(altPaste, SIGNAL(activated()), edit, SLOT(paste()));
		
		Document* document = edit->document();
		undo->setEnabled(document->hasPast());
		redo->setEnabled(document->hasFuture());
		connect(document, SIGNAL(hasPastChanged(bool)), undo, SLOT(setEnabled(bool)));
		connect(document, SIGNAL(hasFutureChanged(bool)), redo, SLOT(setEnabled(bool)));
		
		copy->setEnabled(edit->hasSelection());
		cut->setEnabled(edit->hasSelection());
		connect(edit, SIGNAL(hasSelectionChanged(bool)), copy, SLOT(setEnabled(bool)));
		connect(edit, SIGNAL(hasSelectionChanged(bool)), cut, SLOT(setEnabled(bool)));
		
		QList<QAction*> actions = edit->actions();
		for (int i = 0, n = actions.count(); i < n; ++i)
			actions.at(i)->setShortcutContext(Qt::WidgetShortcut);
	}
	
	QMenu* menu = new QMenu("&Edit", parent);
	menu->addAction(undo);
	menu->addAction(redo);
	menu->addSeparator();
	menu->addAction(copy);
	menu->addAction(cut);
	menu->addAction(paste);
	menu->addSeparator();
	menu->addAction(selectWord);
	menu->addAction(selectLine);
	menu->addAction(selectAll);
	menu->addSeparator();
	menu->addAction(duplicateLine);
	
	return menu;
}

Ref<Context> Edit::context()
{
	if (!context_) context_ = new Context(this);
	return context_;
}

} // namespace pte
