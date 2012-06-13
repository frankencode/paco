// #include <QDebug> // DEBUG
// #include <ftl/stdio> // DEBUG
#include <ftl/ThreadFactory.hpp>
#include "Highlighter.hpp"
#include "InsightModel.hpp"
#include "Document.hpp"

namespace pte
{

Document::Document(int initialLines)
	: cache_(new Cache),
	  defaultStyle_(new Style(Qt::black, Qt::white)),
	  hasPast_(false),
	  hasFuture_(false),
	  timeSaved_(0),
	  yaInvalid_(0),
	  historyEnabled_(false),
	  emissionEnabled_(true),
	  tabWidth_(4),
	  maxLineLength_(65536),
	  maxLineLengthExceeded_(false)
{
	cache_->enableIndexCaching();
	for (int i = 0; i < initialLines; ++i)
		cache_->pushBack(new Line(tabWidth_));
}

Document::~Document()
{
	if (highlighter_) {
		// debug("~Document(): this = %%, Shutting down the highlighter...\n", this);
		if (highlighter_->isRunning()) {
			highlighter_->shutdown();
			highlighter_->wait();
			// debug("~Document(): this = %%, Shutting down the highlighter: done\n", this);
		}
	}
}

Ref<Cache> Document::cache() const { return cache_; }

int Document::numberOfLines() const { return cache_->length(); }

int Document::lineNumber(int lineIndex) const {
	return (matchStatus_) ? matchStatus_->matchingLines_->get(lineIndex)->y_ : lineIndex;
}

Ref<Style> Document::defaultStyle() const { return defaultStyle_; }
void Document::setDefaultStyle(Ref<Style> style) { defaultStyle_ = style; }

int Document::tabWidth() const { return tabWidth_; }
void Document::setTabWidth(int value) { tabWidth_ = value; }
#ifdef QT_MAC_USE_COCOA
int Document::tabWidth(int x) const { return tabWidth_ - (x % tabWidth_); }
#endif

void Document::updateTabWidths(int y)
{
	Ref<Line> line = cache_->get(y);
	
	if (line->tabWidth_ != tabWidth_)
	{
		line->tabWidth_ = tabWidth_;
		
		Ref<ChunkList> chunkList = line->chunkList_;
		int dx = 0;
		Ref<MatchList> matchList = line->matches_;
		Ref<LinkList> linkList = line->linkList_;
		int j = 0, m = (matchList) ? matchList->length() : 0;
		int k = 0, u = (linkList) ? linkList->length() : 0;
		
		for (int i = 0, n = chunkList->length(); i < n; ++i)
		{
			Ref<Chunk> chunk = chunkList->get(i);
			
			if (matchStatus_) {
				// emperic logic, HACK (may not work on matching tabs)
				while (j < m) {
					Match match = matchList->get(j);
					// qDebug() << "shift " << dx << " match " << j << "(" << match.x0_ << "," << match.x1_ << ")";
					if ((chunk->x0_ <= match.x0_) && (match.x0_ < chunk->x1_)) {
						int len = match.x1_ - match.x0_;
						match.x0_ += dx;
						match.x1_ = match.x0_ + len;
						matchList->set(j, match);
						++j;
					}
					else
						break;
				}
				while (k < u) {
					Ref<Link> link = linkList->get(k);
					// qDebug() << "shift " << dx << " link " << k << "(" << link->x0_ << "," << link->x1_ << ")";
					if ((chunk->x0_ <= link->x0_) && (link->x0_ < chunk->x1_)) {
						int len = link->x1_ - link->x0_;
						link->x0_ += dx;
						link->x1_ = link->x0_ + len;
						++k;
					}
					else
						break;
				}
			}
			
			int h = chunk->x1_;
			chunk->x0_ += dx;
			chunk->x1_ += dx;
			if (chunk->text_ == "\t") {
				chunk->x1_ = chunk->x0_ + line->tabWidth(chunk->x0_);
				dx = chunk->x1_ - h;
			}
		}
	}
}

bool Document::historyEnabled() const { return historyEnabled_; }
void Document::setHistoryEnabled(bool on) {
	historyEnabled_ = on;
	if (historyEnabled_ && (!past_)) {
		past_ = new Log;
		future_ = new Log;
	}
}

bool Document::emissionEnabled() const { return emissionEnabled_; }
void Document::setEmissionEnabled(bool on) { emissionEnabled_ = on; }

Delta* Document::newBegin(QString viewState)
{
	Delta* delta = new Delta;
	delta->document_ = this;
	delta->type_ = Delta::Begin;
	delta->viewState_ = viewState;
	return delta;
}

Delta* Document::newEnd(QString viewState)
{
	Delta* delta = new Delta;
	delta->document_ = this;
	delta->type_ = Delta::End;
	delta->viewState_ = viewState;
	return delta;
}

Delta* Document::newDelta(int type, int y, int x0, int x1, QString text)
{
	Delta* delta = new Delta;
	delta->document_ = this;
	delta->type_ = type;
	delta->y_ = y;
	delta->x0_ = x0;
	delta->x1_ = x1;
	delta->tw_ = tabWidth_;
	delta->text_ = text;
	return delta;
}

void Document::begin(QString viewState)
{
	if (historyEnabled_)
	{
		bool isModified2 = isModified();
		
		if (timeSaved_ > past_->length()) {
			timeSaved_ = -1;
		}
		future_->clear();
		past_->pushBack(newBegin(viewState));
		
		if (isModified() != isModified2)
			emit isModifiedChanged(isModified());
	}
}

void Document::end(QString viewState)
{
	if (historyEnabled_) {
		past_->pushBack(newEnd(viewState));
		
		if (hasFuture_ || (!hasPast_)) {
			setHasPast(true);
			setHasFuture(false);
		}
	}
}

int Document::classifyLastDelta(int* y0, int* y1) const
{
	*y0 = intMax;
	*y1 = 0;
	int set = 0;
	if (historyEnabled_) {
		if (past_->length() > 1) {
			int i = 2;
			while (true) {
				Ref<Delta> delta = past_->get(-i);
				if (delta->type_ == Delta::Begin) break;
				if (*y0 > delta->y_) *y0 = delta->y_;
				if (*y1 < delta->y_) *y1 = delta->y_;
				set |= delta->type_;
				++i;
			}
		}
	}
	return set;
}

void Document::pushLine(int y)
{
	cache_->insert(y, new Line(tabWidth_));
	if (y < yaInvalid_)
		yaInvalid_ = y;
	if (historyEnabled_)
		past_->pushBack(newDelta(Delta::PushLine, y));
	if (emissionEnabled_)
		emit changed(newDelta(Delta::PushLine, y));
}

void Document::popLine(int y)
{
	cache_->remove(y);
	if (y < yaInvalid_)
		yaInvalid_ = y;
	if (historyEnabled_)
		past_->pushBack(newDelta(Delta::PopLine, y));
	if (emissionEnabled_)
		emit changed(newDelta(Delta::PopLine, y));
}

void Document::pushSpan(int y, int x0, int* x1, const QString& text, Ref<Style> style, Ref<Token> token)
{
	if (text.length() == 0) { // paranoid HACK
		if (x1) *x1 = x0;
		return;
	}
	
	// book 22/86
	
	if (y < 0) y += cache_->length();
	
	Ref<Line> line = cache_->get(y);
	Ref<ChunkList> chunkList = line->chunkList_;
	
	int ci = breakChunk(y, x0);
	
	if (!style) {
		if (ci > 0)
			style = chunkList->get(ci - 1)->style_;
	}
	/*if (!style) {
		if ((ci == 0) && (y > 0)) {
			int yp = y;
			while (yp > 0) {
				--yp;
				Ref<ChunkList> chunkList = cache_->get(yp)->chunkList_;
				if (chunkList->length() > 0) {
					style = chunkList->get(-1)->style_;
					break;
				}
			}
		}
	}*/
	if (!style) {
		if (ci < chunkList->length() - 1)
			style = chunkList->get(ci + 1)->style_;
		else
			style = defaultStyle();
	}
	
	int h = x0;
	if (!x1) x1 = &h;
	pushChunk(y, ci, x0, x1, text, style, token);
	
	if (historyEnabled_)
		past_->pushBack(newDelta(Delta::PushSpan, y, x0, *x1, text));
	if (emissionEnabled_)
		emit changed(newDelta(Delta::PushSpan, y, x0, *x1, text));
	
	// enforce maximum line length
	if (x0 > maxLineLength_) {
		int h = maxLineLength_, h2;
		maxLineLength_ = intMax;
		popSpan(y, h, intMax);
		pushSpan(y, h, &h2, QString("<Exceeding maximum line length of %1 characters>").arg(h));
		maxLineLengthExceeded_ = true;
		maxLineLength_ = h;
	}
}

void Document::popSpan(int y, int x0, int x1, QString* text)
{
	// book 22/33
	
	if (y < 0) y += cache_->length();
	
	QString text2;
	if ((historyEnabled_ || emissionEnabled_) && (!text))
		text = &text2;
	
	Ref<Line> line = cache_->get(y);
	Ref<ChunkList> chunkList = line->chunkList_;
	
	if (text)
		*text = QString();
	
	int dx = x0 - x1;
	int i = 0;
	while (i < chunkList->length())
	{
		Ref<Chunk> chunk = chunkList->get(i);
		
		if ((x0 <= chunk->x0_) && (chunk->x1_ <= x1)) // completely in
		{
			if (text) text->append(chunk->text_);
			chunkList->remove(i);
			continue;
		}
		else if ((chunk->x0_ < x0) && (x1 < chunk->x1_)) // middle in
		{
			if (text) text->append(chunk->text_.mid(x0 - chunk->x0_, x1 - x0));
			chunk->text_ = chunk->text_.left(x0 - chunk->x0_) + chunk->text_.right(chunk->x1_ - x1);
			chunk->x1_ -= x1 - x0;
		}
		else if ((chunk->x0_ < x1) && (x1 < chunk->x1_)) // left in
		{
			if (text) text->append(chunk->text_.left(x1 - chunk->x0_));
			chunk->text_ = chunk->text_.right(chunk->x1_ - x1);
			chunk->x0_ = x0;
			chunk->x1_ -= x1 - x0;
		}
		else if ((chunk->x0_ < x0) && (x0 < chunk->x1_)) // right in
		{
			if (text) text->append(chunk->text_.right(chunk->x1_ - x0));
			chunk->text_ = chunk->text_.left(x0 - chunk->x0_);
			chunk->x1_ = x0;
		}
		else if (x1 <= chunk->x0_) // outside, shift left
		{
			chunk->x0_ += dx;
			chunk->x1_ += dx;
			if (chunk->text_.at(0) == '\t') {
				int h1 = chunk->x0_ + 1;
				while ((h1 % line->tabWidth_) != 0)
					++h1;
				dx += h1 - chunk->x1_;
				chunk->x1_ = h1;
			}
		}
		else ; // outside
		
		++i;
	}
	
	line->dirty_ = true;
	if (y < yaInvalid_) yaInvalid_ = y;
	
	if (historyEnabled_)
		past_->pushBack(newDelta(Delta::PopSpan, y, x0, x1, *text));
	if (emissionEnabled_)
		emit changed(newDelta(Delta::PopSpan, y, x0, x1, *text));
}

int Document::lineLength(int y)
{
	Ref<Line> line = cache_->get(y);
	Ref<ChunkList> chunkList = line->chunkList_;
	if (!chunkList) return 0;
	if (chunkList->length() == 0) return 0;
	return chunkList->get(-1)->x1_;
}

bool Document::hasPast() const { return hasPast_; }
bool Document::hasFuture() const { return hasFuture_; }

bool Document::hasHistory(int dir) const
{
	return (dir == Undo) ? hasPast_ : hasFuture_;
}

QString Document::stepHistory(int dir, bool forget)
{
	check(historyEnabled_ == true);
	check(hasHistory(dir));
	
	bool isModified2 = isModified();
	
	QString viewState;
	
	if (dir == Undo) {
		// qDebug() << "past_->get(-1)->type_ = " << past_->get(-1)->type_;
		check(past_->get(-1)->type_ == Delta::End);
	}
	else
		check(future_->get(0)->type_ == Delta::Begin);
	
	historyEnabled_ = false;
	
	while (true)
	{
		Ref<Delta, Owner> delta;
		 
		if (dir == Undo) {
			delta = past_->popBack();
			if (!forget) future_->pushFront(delta);
		}
		else {
			delta = future_->popFront();
			if (!forget) past_->pushBack(delta);
		}
		
		if (delta->type_ == Delta::Begin) {
			if (dir == Undo) {
				viewState = delta->viewState_;
				break;
			}
		}
		else if (delta->type_ == Delta::End) {
			if (dir == Redo) {
				viewState = delta->viewState_;
				break;
			}
		}
		else if (delta->type_ == Delta::PushLine) {
			if (dir == Undo)
				popLine(delta->y_);
			else
				pushLine(delta->y_);
		}
		else if (delta->type_ == Delta::PopLine) {
			if (dir == Undo)
				pushLine(delta->y_);
			else
				popLine(delta->y_);
		}
		else {
			int tabWidthSaved = tabWidth_;
			tabWidth_ = delta->tw_;
			updateTabWidths(delta->y_);
			if (delta->type_ == Delta::PushSpan) {
				if (dir == Undo)
					popSpan(delta->y_, delta->x0_, delta->x1_);
				else
					pushSpan(delta->y_, delta->x0_, 0, delta->text_);
			}
			else if (delta->type_ == Delta::PopSpan) {
				if (dir == Undo)
					pushSpan(delta->y_, delta->x0_, 0, delta->text_);
				else
					popSpan(delta->y_, delta->x0_, delta->x1_);
			}
			tabWidth_ = tabWidthSaved;
		}
	}
	
	historyEnabled_ = true;
	
	if (((past_->length() > 0) != hasPast_) || ((future_->length() > 0) != hasFuture_)) {
		setHasPast((past_->length() > 0));
		setHasFuture((future_->length() > 0));
	}
	
	if (isModified() != isModified2)
		emit isModifiedChanged(isModified());
	
	return viewState;
}

void Document::push(int ya, int xa, int* yb, int* xb, const QString& text)
{
	int y = ya, x = xa;
	QStringList l = text.split('\n', QString::KeepEmptyParts);
	
	for (int k = 0, n = l.size(); k < n; ++k)
	{
		QString s(l.at(k));
		if (!s.isEmpty())
			pushSpan(y, x, &x, s);
		if (k != n - 1) {
			++y;
			pushLine(y);
			QString s = copy(y - 1, x, y - 1, intMax);
			if (s.length() > 0) {
				pushSpan(y, 0, 0, s);
				popSpan(y - 1, x, intMax);
			}
			x = 0;
		}
	}
	
	if (yb) *yb = y;
	if (xb) *xb = x;
}

void Document::pop(int ya, int xa, int yb, int xb, QString* text)
{
	if (text)
		*text = copy(ya, xa, yb, xb);
	
	if (ya < yb)
	{
		if (xa > 0) {
			QString s = copy(ya, 0, ya, xa);
			popSpan(ya, 0, xa);
			pushSpan(yb, xb, 0, s);
		}
		for (int y = ya; y <= yb; ++y)
		{
			int x1 = (y == yb) ? xb : intMax;
			popSpan(y, 0, x1);
			if (x1 == intMax) {
				popLine(y);
				--y;
				--ya;
				--yb;
			}
		}
	}
	else {
		popSpan(ya, xa, xb);
	}
}

void Document::getCell(int y, int x, QString* ch, Ref<Style>* style, Ref<Token>* token)
{
	if (y < 0) y += cache_->length();
	
	if (ch) *ch = QString();
	if (style) *style = defaultStyle();
	if (token) *token = 0;
	
	if ((0 <= y) && (y < cache_->length()))
	{
		Ref<Line> line = cache_->get(y);
		Ref<ChunkList> chunkList = line->chunkList_;
		
		for (int i = 0; i < chunkList->length(); ++i)
		{
			Ref<Chunk> chunk = chunkList->get(i);
			if ((chunk->x0_ <= x) && (x < chunk->x1_)) {
				if (ch) {
					if (chunk->text_ == "\t")
						*ch = "\t";
					else
						*ch = chunk->text_.mid(x - chunk->x0_, 1);
				}
				if (style) *style = chunk->style_;
				if (token) *token = chunk->token_;
				break;
			}
		}
	}
}

QString Document::getChar(int y, int x)
{
	QString ch;
	getCell(y, x, &ch);
	if (ch == QString()) ch = " ";
	return ch;
}

QString Document::copySpan(int y, int x0, int x1, char gap, bool replaceTabBySpace)
{
	QStringList l;
	
	if (y < 0) y += cache_->length();
	
	if ((y < 0) || (cache_->length()) <= y) return QString();
	
	if (replaceTabBySpace)
		updateTabWidths(y);
	
	Ref<Line> line = cache_->get(y);
	Ref<ChunkList> chunkList = line->chunkList_;
	
	int x = x0;
	
	for (int k = 0; k < chunkList->length(); ++k)
	{
		Ref<Chunk> chunk = chunkList->get(k);
		
		QString fragment;
		
		if ((x0 <= chunk->x0_) && (chunk->x1_ <= x1)) // completely in
			fragment = chunk->text_;
		else if ((chunk->x0_ < x0) && (x1 < chunk->x1_)) // middle in
			fragment = chunk->text_.mid(x0 - chunk->x0_, x1 - x0);
		else if ((chunk->x0_ < x1) && (x1 < chunk->x1_)) // left in
			fragment = chunk->text_.left(x1 - chunk->x0_);
		else if ((chunk->x0_ < x0) && (x0 < chunk->x1_)) // right in
			fragment = chunk->text_.right(chunk->x1_ - x0);
		else ; // outside
		
		if (fragment.length() > 0) {
			if (x < chunk->x0_) {
				l.append(QString(chunk->x0_ - x, gap));
				x = chunk->x0_;
			}
			if (replaceTabBySpace)
				if (fragment.at(0) == '\t')
					fragment = QString(line->tabWidth(x), ' ');
			l.append(fragment);
			x += chunk->x1_ - chunk->x0_;
		}
	}
	
	if ((x < x1) && (x1 < intMax))
		l.append(QString(x1 - x, gap));
	
	return l.join(QString());
}

QString Document::copy(int ya, int xa, int yb, int xb)
{
	QStringList l;
	
	if (ya < 0)
		ya = 0;
	if (yb >= cache_->length())
		yb = cache_->length() - 1;
	
	for (int y = ya; y <= yb; ++y)
	{
		int x0 = (y == ya) ? xa : 0;
		int x1 = (y == yb) ? xb : intMax;
		l.append(copySpan(y, x0, x1));
	}
	
	return l.join("\n");
}

/*QString Document::replaceTabBySpace(QString s, int x0) const // obsolete method
{
	int i = 0, n = s.size(), x = x0;
	QStringList l;
	while (i < n) {
		int k = i;
		while (k < n) {
			if (s.at(k) == '\t') break;
			++k;
		}
		if (i < k) {
			QString h = s.mid(i, k-i);
			l.append(h);
			x += h.size();
		}
		if (k != n) {
			QString h(tabWidth(x), ' ');
			l.append(h);
			x += h.size();
		}
		i = k + 1;
	}
	return l.join(QString());
}*/

void Document::load(QTextStream* source)
{
	highlighterYield();
	
	bool isModifiedSaved = isModified();
	
	if (past_) {
		past_->clear();
		future_->clear();
	}
	
	bool historyEnabledSaved = historyEnabled_;
	bool emissionEnabledSaved = emissionEnabled_;
	historyEnabled_ = false;
	emissionEnabled_ = false;
	
	maxLineLengthExceeded_ = false;
	cache_->clear();
	while (true) {
		QString s = source->readLine();
		if (s.isNull()) break;
		pushLine(cache_->length());
		pushSpan(cache_->length() - 1, 0, 0, s, defaultStyle());
	}
	//if (cache_->length() == 0)
	//	pushLine(0);
	pushLine(cache_->length());
	
	yaInvalid_ = 0;
	
	historyEnabled_ = historyEnabledSaved;
	emissionEnabled_ = emissionEnabledSaved;
	
	if (historyEnabled_)
		timeSaved_ = 0;
	
	if (isModified() != isModifiedSaved)
		emit isModifiedChanged(false);
	
	highlighterRestart();
	// highlighterSync(0.5);
	highlighterResume();
}

void Document::save(QTextStream* sink)
{
	highlighterYield();
	
	int trailingEmptyLines = 0;
	for (int y = cache_->length() - 1; y >= 0; --y) {
		Ref<Line> line = cache_->get(y);
		if (line->chunkList_->length() != 0) break;
		++trailingEmptyLines;
	}
	
	for (int y = 0; y < cache_->length() - trailingEmptyLines; ++y) {
		QString text = copySpan(y, 0, intMax);
		int tw = 0; // trailing whitespace
		while (tw < text.length()) {
			if (text.at(text.length() - tw - 1).isSpace()) ++tw;
			else break;
		}
		if (tw > 0) text = text.left(text.length() - tw);
		*sink << text << '\n'; // HACK, line ending style should be selectable
	}
	
	if (historyEnabled_) {
		bool isModified2 = isModified();
		timeSaved_ = past_->length();
		if (isModified2)
			emit isModifiedChanged(false);
	}
	
	highlighterResume();
}

bool Document::isModified() const
{
	if (!past_) return true;
	return timeSaved_ != past_->length();
}

void Document::markDirty()
{
	bool isModifiedOld = isModified();
	timeSaved_ = -1;
	if (isModifiedOld != isModified())
		emit isModifiedChanged(isModified());
}

QString Document::text()
{
	QString text;
	QTextStream stream(&text);
	save(&stream);
	return text;
}

void Document::setText(const QString& text)
{
	QString text2 = text;
	QTextStream stream(&text2);
	load(&stream);
}

QString Document::title() const { return title_; }
void Document::setTitle(QString value) { title_ = value; }

QString Document::filePath() const { return filePath_; }
void Document::setFilePath(QString value) { filePath_ = value; }

QFileInfo Document::fileInfo() {
	if (fileInfo_.filePath() != filePath_)
		fileInfo_ = QFileInfo(filePath_);
	return fileInfo_;
}

int Document::maxLineLength() const { return maxLineLength_; }
void Document::setMaxLineLength(int value) { maxLineLength_ = value; }

bool Document::maxLineLengthExceeded() const { return maxLineLengthExceeded_; }

Ref<Highlighter> Document::highlighter() const { return highlighter_; }
void Document::setHighlighter(Ref<Highlighter> highlighter) {
	if (highlighter_ == highlighter) return;
	yaInvalid_ = 0;
	if (highlighter_) {
		disconnect(highlighter_, SIGNAL(ready()), this, SIGNAL(highlighterReady()));
		if (highlighter_->isRunning()) {
			highlighter_->shutdown();
			highlighter_->wait();
		}
	}
	highlighter_ = highlighter;
	if (highlighter_) {
		setDefaultStyle(highlighter_->defaultStyle());
		connect(highlighter_, SIGNAL(ready()), this, SIGNAL(highlighterReady()));
		if (!highlighter_->isRunning())
			highlighter_->start();
	}
}

int Document::yaInvalid() const { return yaInvalid_; }
void Document::setYaInvalid(int ya) { yaInvalid_ = ya; }

int Document::breakChunk(int y, int x)
{
	Ref<ChunkList> chunkList = cache_->get(y)->chunkList_;
	
	int i = chunkList->length() - 1;
	
	while (i >= 0)
	{
		Ref<Chunk> chunk = chunkList->get(i);
		
		if (chunk->x0_ < x)
		{
			if (x < chunk->x1_)
			{
				Ref<Chunk> chunk2 =
					new Chunk(
						chunk->text_.right(chunk->x1_ - x),
						x,
						chunk->x1_,
						chunk->style_,
						chunk->token_
					);
				
				chunkList->insert(i + 1, chunk2);
				
				chunk->text_ = chunk->text_.left(x - chunk->x0_);
				chunk->x1_ = x;
			}
			
			++i;
			
			break;
		}
		
		--i;
	}
	
	if (i == -1) i = 0;
	
	return i;
}

void Document::pushChunk(int y, int ci, int x0, int* x1, QString text, Ref<Style> style, Ref<Token> token)
{
	Ref<Line> line = cache_->get(y);
	Ref<ChunkList> chunkList = line->chunkList_;
	if (!style) style = defaultStyle();
	
	// insert new text chunks (tabulator chars placed on separate chunks)
	QStringList l = text.split('\t', QString::KeepEmptyParts);
	int x = x0;
	for (int k = 0, n = l.size(); k < n; ++k) {
		QString s(l.at(k));
		if (s.length() > 0) {
			chunkList->insert(ci, new Chunk(s, x, x + s.length(), style, token));
			x += s.length();
			++ci;
		}
		if (k != n - 1) {
			int x2 = x + line->tabWidth(x);
			chunkList->insert(ci, new Chunk("\t", x, x2, style, token));
			x = x2;
			++ci;
		}
	}
	
	if (x1) *x1 = x;
	
	// shift right
	{
		int dx = x - x0;
		while (ci < chunkList->length()) {
			Ref<Chunk> chunk = chunkList->get(ci);
			chunk->x0_ += dx;
			chunk->x1_ += dx;
			if (chunk->text_.at(0) == '\t') {
				int h1 = chunk->x0_ + 1;
				while ((h1 % line->tabWidth_) != 0)
					++h1;
				dx += h1 - chunk->x1_;
				chunk->x1_ = h1;
			}
			++ci;
		}
	}
	
	line->dirty_ = true;
	if (y < yaInvalid_) yaInvalid_ = y;
}

bool Document::mapCursorToContentPos(int cy, int cx, int* k_, int* i_)
{
	Ref<Line> line = cache_->get(cy);
	Ref<ChunkList> chunkList = line->chunkList_;
	Ref<Chunk> chunk;
	int k = 0;
	int i = -1;
	while (k < chunkList->length()) {
		chunk = chunkList->get(k);
		if ((chunk->x0_ <= cx) && (cx < chunk->x0_ + chunk->text_.length()))
		{
			i = cx - chunk->x0_;
			break;
		}
		
		if ((cx == chunk->x1_) && (k == chunkList->length() - 1)) {
			++k;
			i = 0;
			break;
		}
		
		++k;
	}
	
	if ((chunkList->length() == 0) && (cx == 0))
		i = 0;
	
	*k_ = k;
	*i_ = i;
	
	return i != -1;
}

void Document::stepBackward(int* cy, int* cx)
{
	int k, i;
	if (mapCursorToContentPos(*cy, *cx, &k, &i))
	{
		if (i > 0) {
			--(*cx);
		}
		else {
			if (k > 0) {
				Ref<Chunk> chunk = cache_->get(*cy)->chunkList_->get(k - 1);
				*cx = chunk->x0_ + chunk->text_.length() - 1;
			}
			else {
				if (*cy > 0) {
					--(*cy);
					Ref<ChunkList> chunkList = cache_->get(*cy)->chunkList_;
					if (chunkList->length() > 0) {
						Ref<Chunk> chunk = chunkList->get(chunkList->length() - 1);
						*cx = chunk->x1_;
					}
					else
						*cx = 0;
				}
			}
		}
	}
}

void Document::stepForward(int* cy, int* cx)
{
	int k, i;
	if (mapCursorToContentPos(*cy, *cx, &k, &i))
	{
		Ref<ChunkList> chunkList = cache_->get(*cy)->chunkList_;
		if (k < chunkList->length()) {
			Ref<Chunk> chunk = chunkList->get(k);
			if (i < chunk->text_.length() - 1) {
				++(*cx);
			}
			else if ((i == chunk->text_.length() - 1) && (k == chunkList->length() - 1)) {
				*cx = chunk->x1_;
			}
			else {
				*cx = chunkList->get(k + 1)->x0_;
			}
		}
		else {
			if (*cy < cache_->length() - 1) {
				++(*cy);
				*cx = 0;
			}
		}
	}
}

void Document::stepNearest(int* cy, int* cx)
{
	if (*cy < 0) *cy = 0;
	if (*cy >= cache_->length()) *cy = cache_->length() - 1;
	
	Ref<ChunkList> chunkList = cache_->get(*cy)->chunkList_;
	
	int cx2 = 0;
	for (int k = 0; k < chunkList->length(); ++k) {
		Ref<Chunk> chunk = chunkList->get(k);
		if (chunk->x0_ <= *cx) {
			if (*cx < chunk->x0_ + chunk->text_.length()) {
				cx2 = *cx;
				break;
			}
			else if (*cx < chunk->x1_) {
				cx2 = chunk->x0_;
				break;
			}
			else
				cx2 = chunk->x1_;
		}
	}
	
	*cx = cx2;
}

bool Document::beginOfDocument(int cy, int cx) const
{
	return (cy == 0) && (cx == 0);
}

bool Document::endOfDocument(int cy, int cx) const
{
	if (cy == cache()->length() - 1) {
		Ref<Line> lastLine = cache()->get(cache()->length() - 1);
		if (lastLine->chunkList_->length() > 0)
			return cx >= lastLine->chunkList_->get(lastLine->chunkList_->length() -1)->x1_;
		return true;
	}
	return false;
}

void Document::setHasPast(bool value)
{
	bool h = hasPast_;
	hasPast_ = value;
	if (h != value)
		emit hasPastChanged(value);
}

void Document::setHasFuture(bool value)
{
	bool h = hasFuture_;
	hasFuture_ = value;
	if (h != value)
		emit hasFutureChanged(value);
}

Ref<MatchStatus, Owner> Document::find(QRegExp pattern)
{
	return findAndReplace(pattern, false);
}

Ref<MatchStatus, Owner> Document::replace(QRegExp pattern, QString substitute, int* cy, int* cx)
{
	return findAndReplace(pattern, true, substitute, cy, cx);
}

Ref<MatchStatus> Document::matchStatus() const { return matchStatus_; }

Ref<InsightModel> Document::insightModel() const { return insightModel_; }
void Document::setInsightModel(Ref<InsightModel> model) { insightModel_ = model; }

void Document::matchStatusYield(Ref<MatchStatus> status, bool sync)
{
	Ref<Document> doc;
	for (int i = 0, n = status->matchingLines_->length(); i < n; ++i) {
		Ref<MatchingLine> ml = status->matchingLines_->get(i);
		if (doc != ml->document_) {
			// debug("matchStatusYield(): %%\n", ml->document_->filePath().toUtf8().data());
			ml->document_->highlighterYield();
			if (sync) {
				// debug("matchStatusYield(): highlighterSync(): %%\n", ml->document_->filePath().toUtf8().data());
				ml->document_->highlighterSync(); // quick HACK, is inefficient!
				// debug("matchStatusYield(): voila\n");
			}
		}
		doc = ml->document_;
	}
}

void Document::matchStatusResume(Ref<MatchStatus> status)
{
	Ref<Document> doc;
	for (int i = 0, n = status->matchingLines_->length(); i < n; ++i) {
		Ref<MatchingLine> ml = status->matchingLines_->get(i);
		if (doc != ml->document_) {
			ml->document_->highlighterResume();
			// qDebug() << "matchStatusResume: " << ml->document_->filePath();
		}
		doc = ml->document_;
	}
}

Ref<Document, Owner> Document::matchStatusDocument(Ref<MatchStatus> status)
{
	Ref<Document, Owner> mdoc = new Document(0);
	mdoc->matchStatus_ = status;
	mdoc->emissionEnabled_ = false;
	
	matchStatusYield(status, true /*sync*/);
	
	for (int i = 0, n = status->matchingLines_->length(); i < n; ++i) {
		Ref<MatchingLine> ml = status->matchingLines_->get(i);
		if (ml->y_ < 0)
			mdoc->cache_->append(new Line(4));
		else
			mdoc->cache_->append(ml->document_->cloneLine(ml->y_));
	}
	
	matchStatusResume(status);
	
	return mdoc;
}

Ref<Line, Owner> Document::cloneLine(int y)
{
	Ref<Line> line = cache_->get(y);
	Ref<Line, Owner> clone = new Line(line->tabWidth_);
	for (int i = 0, n = line->chunkList_->length(); i < n; ++i) {
		Ref<Chunk> chunk = line->chunkList_->get(i);
		clone->chunkList_->append(new Chunk(chunk->text_, chunk->x0_, chunk->x1_, chunk->style_, chunk->token_));
	}
	clone->matches_ = line->matches_;
	return clone;
}

Ref<MatchList, Owner> Document::matchLine(int y, QRegExp pattern)
{
	Ref<MatchList, Owner> matchList;
	
	QString text = copySpan(y, 0, intMax, ' ', true);
	
	int x = 0;
	while (x < text.length()) {
		x = pattern.indexIn(text, x);
		if (x == -1) break;
		int len = pattern.matchedLength();
		if (len == 0) break; // workaround HACK
		if (!matchList)
			matchList = new MatchList;
		matchList->append(Match(x, x + len));
		x += len;
	}
	
	return matchList;
}

Ref<MatchStatus, Owner> Document::findAndReplace(QRegExp pattern, bool replace, QString substitute, int* cy, int* cx)
{
	highlighterYield();
	
	Ref<MatchingLines, Owner> matchingLines = new MatchingLines;
	int yMax = 0;
	int numMatches = 0;
	
	for (int y = 0, nl = cache_->length(); y < nl; ++y)
	{
		Ref<Line> line = cache_->get(y);
		Ref<MatchList, Owner> matchList = matchLine(y, pattern);
		line->matches_ = matchList;
		if (matchList) {
			matchingLines->append(new MatchingLine(this, y, matchList));
			numMatches += matchList->length();
			yMax = y;
			if (replace) {
				bool emissionEnabledSaved = emissionEnabled_;
				emissionEnabled_ = false;
				int shift = 0;
				for (int i = 0, nm = matchList->length(); i < nm; ++i) {
					Match match = matchList->get(i);
					int x0 = match.x0_ + shift;
					int x1 = match.x1_ + shift;
					int x1b = 0;
					popSpan(y, x0, x1);
					pushSpan(y, x0, &x1b, substitute);
					shift = x1b - match.x1_;
					match.x0_ = x0;
					match.x1_ = x1b;
					matchList->set(i, match);
				}
				if ((cy) && (cx))
					if (*cy == y)
						*cx += shift;
				emissionEnabled_ = emissionEnabledSaved;
			}
		}
	}
	
	highlighterResume();
	
	return new MatchStatus(
		replace ? QRegExp(substitute, Qt::CaseSensitive, QRegExp::FixedString) : pattern,
		matchingLines,
		yMax,
		(numMatches > 0),
		numMatches
	);
}

void Document::updateMatchStatusDocument(Ref<Document> mdoc, Ref<Delta> delta, Ref<Highlighter> highlighter)
{
	if (delta)
	{
		matchStatusYield(mdoc->matchStatus_, false /*sync*/);
		
		if (delta->type_ == Delta::PushLine)
		{
			Ref<MatchingLines> matchingLines = mdoc->matchStatus_->matchingLines_;
			for (int i = 0, n = matchingLines->length(); i < n; ++i) {
				Ref<MatchingLine> ml = matchingLines->get(i);
				if (delta->document_ == ml->document_) {
					if (ml->y_ >= delta->y_) {
						++ml->y_;
						if (mdoc->matchStatus_->yMax_ < ml->y_)
							mdoc->matchStatus_->yMax_ = ml->y_;
					}
				}
			}
		}
		else if (delta->type_ == Delta::PopLine)
		{
			Ref<MatchingLines> matchingLines = mdoc->matchStatus_->matchingLines_;
			for (int i = 0, n = matchingLines->length(); i < n; ++i) {
				Ref<MatchingLine> ml = matchingLines->get(i);
				if (delta->document_ == ml->document_) {
					if (ml->y_ == delta->y_) {
						matchingLines->remove(i);
						mdoc->cache_->remove(i);
						continue;
					}
					else if (ml->y_ >= delta->y_) {
						--ml->y_;
					}
				}
			}
		}
		else if ( (delta->type_ == Delta::PushSpan) ||
		          (delta->type_ == Delta::PopSpan) )
		{
			Ref<MatchingLines> matchingLines = mdoc->matchStatus_->matchingLines_;
			Ref<Line> line = delta->document_->cache_->get(delta->y_);
			Ref<MatchList, Owner> matchList = delta->document_->matchLine(delta->y_, mdoc->matchStatus_->pattern_);
			line->matches_ = matchList;
			bool found = false;
			int i = 0, n = matchingLines->length();
			int j = -1; // insertion point
			while (i < n) {
				Ref<MatchingLine> ml = matchingLines->get(i);
				if (delta->document_ == ml->document_) {
					if (ml->y_ == delta->y_) {
						if (matchList) {
							ml->matches_ = matchList;
							mdoc->cache_->set(i, delta->document_->cloneLine(delta->y_));
						}
						else {
							matchingLines->remove(i);
							mdoc->cache_->remove(i);
						}
						found = true;
						break;
					}
					if (delta->y_ < ml->y_) {
						j = i;
						break;
					}
					else {
						j = i + 1;
					}
				}
				++i;
			}
			if ((!found) && (matchList) && (j != -1)) {
				Ref<MatchingLine> ml = new MatchingLine(delta->document_, delta->y_, matchList);
				matchingLines->insert(j, ml);
				mdoc->cache_->insert(j, delta->document_->cloneLine(delta->y_));
				if (mdoc->matchStatus_->yMax_ < ml->y_)
					mdoc->matchStatus_->yMax_ = ml->y_;
			}
		}
		
		matchStatusResume(mdoc->matchStatus_);
	}
	
	if (highlighter)
	{
		matchStatusYield(mdoc->matchStatus_, false /*sync*/);
		
		Ref<MatchingLines> matchingLines = mdoc->matchStatus_->matchingLines_;
		for (int i = 0, n = matchingLines->length(); i < n; ++i) {
			Ref<MatchingLine> ml = matchingLines->get(i);
			Ref<Document> doc = ml->document_;
			if ((doc == highlighter->document()) && (0 <= ml->y_) && (ml->y_ < doc->cache_->length()))
				mdoc->cache_->set(i, doc->cloneLine(ml->y_));
		}
		
		matchStatusResume(mdoc->matchStatus_);
	}
}

} // namespace pte
