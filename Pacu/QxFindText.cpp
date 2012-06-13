#include <QLayout>
#include <QScrollBar>
#include <QLineEdit>
#include <QProgressDialog>
#include <QSettings>
#include <QCompleter>
#include <pte/Edit.hpp>
#include <ftl/streams> // DEBUG
#include <ftl/Semaphore.hpp>
#include <ftl/ScopeGuard.hpp>
#include "QxComboBox.hpp"
#include "QxMatchView.hpp"
#include "QxVideoTerminal.hpp"
#include "QxPreferences.hpp"
#include "QxFindText.hpp"

#define PACU_DEBUG_FINDTEXT 0

namespace pacu
{

QxFindText::QxFindText(QxPreferences* preferences, QWidget* parent)
	: QxControl(parent, new QxVisual(styleManager()->style("panel"))),
	  preferences_(preferences),
	  findAndReplaceIsRunning_(new Semaphore(1))
{
	pattern_ = new QxComboBox(this);
	pattern_->setEditable(true);
	pattern_->completer()->setCaseSensitivity(Qt::CaseSensitive);
	
	findButton_ = new QPushButton(tr("Find"), this);
	
	useRegex_ = new QCheckBox(tr("Regex."), this);
	caseSensitive_ = new QCheckBox(tr("Case sens."), this);
	wholeWord_ = new QCheckBox(tr("Whole word"), this);
#ifdef Q_WS_MAC
	useRegex_->setAttribute(Qt::WA_MacSmallSize);
	caseSensitive_->setAttribute(Qt::WA_MacSmallSize);
	wholeWord_->setAttribute(Qt::WA_MacSmallSize);
#endif
	
	replacement_ = new QxComboBox(this);
	replacement_->setEditable(true);
	replacement_->completer()->setCaseSensitivity(Qt::CaseSensitive);
	replacement_->setDisabled(true);
	replaceButton_ = new QPushButton(tr("Replace"), this);
	replaceButton_->setDisabled(true);
	
	scope_ = new QxComboBox(this);
	scope_->addItems(tr("Current File;Current Directory;Terminal").split(";"));
	scope_->setItemData(0, CurrentFile);
	scope_->setItemData(1, CurrentDirectory);
	scope_->setItemData(2, Terminal);
	
	QWidget* statusCarrier = new QWidget(this);
	matchView_ = new QxMatchView(statusCarrier);
	{
		QScrollBar* vScroll = new QScrollBar(Qt::Vertical, statusCarrier);
		matchView_->setVerticalScrollBar(vScroll);
		
		QHBoxLayout* layout = new QHBoxLayout;
		layout->setSpacing(0);
		layout->setMargin(0);
		layout->addWidget(matchView_);
		layout->addWidget(vScroll);
		statusCarrier->setLayout(layout);
	}
	matchView_->setDocument(new Document(0));
	connect(matchView_, SIGNAL(gotoMatch(Ref<Document>, int, int, int)), this, SIGNAL(gotoMatch(Ref<Document>, int, int, int)));
	
	QHBoxLayout* cell1 = new QHBoxLayout;
	cell1->setSpacing(7);
	cell1->setMargin(7);
	cell1->addWidget(pattern_);
	cell1->addWidget(findButton_);
	cell1->addSpacing(4);
	cell1->addWidget(caseSensitive_);
	cell1->addWidget(wholeWord_);
	cell1->addWidget(useRegex_);
	cell1->addSpacing(4);
	cell1->addWidget(replacement_);
	cell1->addWidget(replaceButton_);
	cell1->addSpacing(4);
	cell1->addWidget(scope_);
	// cell1->addStretch();
	
	pattern_->setSizePolicy(QSizePolicy::MinimumExpanding, pattern_->sizePolicy().verticalPolicy());
	replacement_->setSizePolicy(QSizePolicy::MinimumExpanding, replacement_->sizePolicy().verticalPolicy());
	scope_->setSizePolicy(QSizePolicy::Maximum, scope_->sizePolicy().verticalPolicy());
	
	QVBoxLayout* cell0 = new QVBoxLayout;
	cell0->setSpacing(0);
	cell0->setMargin(0);
	cell0->addLayout(cell1);
#ifdef Q_WS_MAC
	cell0->addSpacing(4); // layout HACK
#endif
	cell0->addWidget(styleManager()->hl(this));
	cell0->addWidget(statusCarrier);
	setLayout(cell0);
	
	connect(pattern_->lineEdit(), SIGNAL(returnPressed()), this, SLOT(find()));
	connect(replacement_->lineEdit(), SIGNAL(returnPressed()), this, SLOT(replace()));
	connect(useRegex_, SIGNAL(toggled(bool)), wholeWord_, SLOT(setDisabled(bool)));
	connect(findButton_, SIGNAL(pressed()), pattern_->lineEdit(), SIGNAL(returnPressed()));
	connect(replaceButton_, SIGNAL(pressed()), replacement_->lineEdit(), SIGNAL(returnPressed()));
	
	connect(pattern_, SIGNAL(activated(const QString&)), this, SLOT(patternChanged(const QString&)));
	connect(pattern_, SIGNAL(editTextChanged(const QString&)), this, SLOT(patternChanged(const QString&)));
	connect(scope_, SIGNAL(activated(int)), this, SLOT(scopeChanged(int)));
	
	connect(pattern_, SIGNAL(escape()), this, SIGNAL(escape()));
	connect(replacement_, SIGNAL(escape()), this, SIGNAL(escape()));
	
	setFocusProxy(pattern_);
	
	setTabOrder(pattern_, replacement_);
	setTabOrder(replacement_, caseSensitive_);
	setTabOrder(caseSensitive_, wholeWord_);
	setTabOrder(wholeWord_, useRegex_);
	setTabOrder(useRegex_, matchView_);
}

void QxFindText::saveState(QSettings* settings)
{
	{
		QStringList items;
		for (int i = 0, n = pattern_->count(); i < n; ++i)
			items.append(pattern_->itemText(i));
		settings->setValue("findText/pattern/list", items);
	}
	settings->setValue("findText/pattern/currentIndex", pattern_->currentIndex());
	{
		QStringList items;
		for (int i = 0, n = replacement_->count(); i < n; ++i)
			items.append(replacement_->itemText(i));
		settings->setValue("findText/replacement/list", items);
	}
	settings->setValue("findText/replacement/currentIndex", replacement_->currentIndex());
}

void QxFindText::restoreState(const QSettings* settings)
{
	if (settings->contains("findText/pattern/list"))
		pattern_->addItems(settings->value("findText/pattern/list").toStringList());
	if (settings->contains("findText/pattern/currentIndex"))
		pattern_->setCurrentIndex(settings->value("findText/pattern/currentIndex").toInt());
	if (settings->contains("findText/replacement/list"))
		replacement_->addItems(settings->value("findText/replacement/list").toStringList());
	if (settings->contains("findText/replacement/currentIndex"))
		pattern_->setCurrentIndex(settings->value("findText/replacement/currentIndex").toInt());
}

void QxFindText::setScope(int scope)
{
	scope_->setCurrentIndex(scope);
}

void QxFindText::selectPatternText()
{
	pattern_->setFocus();
	pattern_->lineEdit()->selectAll();
}

void QxFindText::setTextView(View* view) { textView_ = view; }
void QxFindText::setVideoTerminal(QxVideoTerminal* vt) { vt_ = vt; }

void QxFindText::setViewColors(Ref<ViewColors> colors)
{
	dismiss();
	matchView_->setColors(colors);
}

void QxFindText::setViewMetrics(Ref<ViewMetrics> metrics)
{
	bool updateMatches = (matchView_->metrics()->tabWidth_ != metrics->tabWidth_);
	matchView_->setMetrics(metrics);
	
	if ((updateMatches) && (matchStatusDocument_)) {
		// uggly HACK to support variable tab widths
		// (but ensures that all match coords are instantly correct;)
		for (int y = 0, n = matchStatusDocument_->cache()->length(); y < n; ++y)
			matchStatusDocument_->updateTabWidths(y);
	}
}

void QxFindText::terminalPaletteChanged()
{
	dismiss();
	matchView_->update(); // redundant?
}

void QxFindText::updateMatchStatus(Ref<Delta, Owner> delta)
{
	// quick, but safe workaround HACK, needs review
	if (!findAndReplaceIsRunning_->tryAcquire()) return;
	else findAndReplaceIsRunning_->release();
	
	if (!matchStatus_) return;
	if ((scope_->currentIndex() == CurrentFile) || (scope_->currentIndex() == Terminal)) {
		Ref<Document> doc;
		for (int i = 0, n = matchStatus_->matchingLines_->length(); (!doc) && (i < n); ++i)
			doc = matchStatus_->matchingLines_->get(i)->document_;
		if (doc != delta->document_)
			return;
	}
	Document::updateMatchStatusDocument(matchStatusDocument_, delta);
	if (matchView_->hasSelection())
		matchView_->unselect();
	matchView_->updateVerticalScrollRange();
	matchView_->update();
}

void QxFindText::updateMatchStatus()
{
	// quick, but safe workaround HACK, needs review
	if (!findAndReplaceIsRunning_->tryAcquire()) return;
	else findAndReplaceIsRunning_->release();
	
	if (!matchStatus_) return;
	Document* doc = qobject_cast<Document*>(sender());
	if (!doc) return;
	if (!doc->highlighter()) return;
	Document::updateMatchStatusDocument(matchStatusDocument_, 0, doc->highlighter());
	matchView_->update();
}

void QxFindText::patternChanged(const QString&)
{
	bool on = (scope_->currentIndex() != Terminal);
	if (matchStatus_)
		on = on && (matchStatus_->pattern_.pattern() == pattern_->currentText()) &&
		           (matchStatus_->matchingLines_->length() > 0);
	replacement_->setEnabled(on);
	replaceButton_->setEnabled(on);
}

void QxFindText::scopeChanged(int scope)
{
	patternChanged(pattern_->currentText());
	if (scope == Terminal) {
		if (vt_) {
			if (matchView_->colors() != vt_->colors())
				setViewColors(vt_->colors());
		}
		matchView_->setMetrics(preferences_->terminalMetrics());
	}
	else {
		if (textView_) {
			if (matchView_->colors() != textView_->colors())
				setViewColors(textView_->colors());
		}
		matchView_->setMetrics(preferences_->editorMetrics());
	}
}

void QxFindText::find()
{
	if (pattern_->currentText().isEmpty()) {
		dismiss();
		pattern_->setFocus();
	}
	else
		findAndReplace(false);
	
	while (pattern_->count() > 10)
		pattern_->removeItem(0);
}

void QxFindText::replace()
{
	if (pattern_->currentText().isEmpty())
		pattern_->setFocus();
	else
		findAndReplace(true);
	
	while (replacement_->count() > 10)
		replacement_->removeItem(0);
}

static void gatherPaths(QDir dir, QStringList* paths, QProgressDialog* progress)
{
	QStringList files = dir.entryList(QStringList(), QDir::Files|QDir::NoDotAndDotDot|QDir::Readable, QDir::Name);
	for (int i = 0, n = files.size(); i < n; ++i)
		paths->append(dir.filePath(files.at(i)));
	QStringList dirs = dir.entryList(QStringList(), QDir::AllDirs|QDir::NoDotAndDotDot|QDir::Readable|QDir::Executable, QDir::Name);
	progress->setValue(0);
	for (int i = 0, n = dirs.size(); (i < n) && (!progress->wasCanceled()); ++i)
		gatherPaths(dir.filePath(dirs.at(i)), paths, progress);
}

void QxFindText::findAndReplace(bool replace)
{
	dismiss();
	
	ScopeGuard<Semaphore> guard(findAndReplaceIsRunning_);

	QString ms = wholeWord_->isChecked() ? "\\b" + QRegExp::escape(pattern_->currentText()) + "\\b" : pattern_->currentText();
	Qt::CaseSensitivity cs = caseSensitive_->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
	QRegExp::PatternSyntax ps = (useRegex_->isChecked() || wholeWord_->isChecked()) ? QRegExp::RegExp2 : QRegExp::FixedString;
	QRegExp pattern(ms, cs, ps);
	QString substitute = replacement_->currentText();
	int scope = replace ? scopeSaved_ : scope_->currentIndex();
	
	if (!replace)
		scopeSaved_ = scope;
	
	if (scope != scope_->currentIndex())
		scope_->setCurrentIndex(scope);
	
	if ((scope == CurrentFile) || ((scope == Terminal) && (!replace)))
	{
		View* view = 0;
		if (scope == CurrentFile) view = textView_;
		else view = vt_;
		
		if (!view) return;
		
		matchView_->setColors(view->colors());
		matchView_->setMetrics(view->metrics());
		
		Ref<Document> doc = view->document();
		
		if (replace) {
			if (doc->historyEnabled()) doc->begin(view->saveState());
			int cy = view->cursorLine();
			int cx = view->cursorColumn();
			matchStatus_ = doc->replace(pattern, substitute, &cy, &cx);
			view->moveCursor(cy, cx);
			if (doc->historyEnabled()) doc->end(view->saveState());
		}
		else {
			matchStatus_ = doc->find(pattern);
		}
		
		if (matchStatus_->matchingLines_->length() > 0) {
			matchStatus_->matchingLines_->pushFront(
				new MatchingLine(doc, -1, 0)
			);
		}
	}
	else if (scope == CurrentDirectory)
	{
		if ((replace) && (paths_.size() == 0)) return;
		
		QProgressDialog progress(this);
		progress.setMinimumDuration(1000);
		progress.setWindowModality(Qt::WindowModal);
		progress.setLabelText(tr("Gathering directory information..."));
		
		if (!replace) {
			matchStatus_ = 0;
			paths_ = QStringList();
			gatherPaths(QDir::current(), &paths_, &progress);
		}
		
		progress.setLabelText(tr("Searching files..."));
		progress.setRange(0, paths_.size());
		
		for (int i = 0, n = paths_.size(); (i < n) && (!progress.wasCanceled()); ++i)
		{
			int mode = (replace) ? QxFileOpenRequest::CurrentTabWidget : 0;
			QString path = paths_.at(i);
			
			#if PACU_DEBUG_FINDTEXT
			debug("QxFindText::findAndReplace().0: paths_.at(%%) = %%\n", i, paths_.at(i).toUtf8().data());
			#endif
			
			Ref<QxFileOpenRequest, Owner> request = new QxFileOpenRequest(mode, path);
			request->acceptBinary_ = false;
			emit openFile(request);
			
			#if PACU_DEBUG_FINDTEXT
			debug("QxFindText::findAndReplace().1: paths_.at(%%) = %%\n", i, paths_.at(i).toUtf8().data());
			#endif
			
			if (request->failed_) {
				#if PACU_DEBUG_FINDTEXT
				debug("QxFindText::findAndReplace().f: paths_.at(%%) = %%\n", i, paths_.at(i).toUtf8().data());
				#endif
				
				paths_.removeAt(i);
				--i;
				--n;
			}
			else {
				#if PACU_DEBUG_FINDTEXT
				debug("QxFindText::findAndReplace().2: paths_.at(%%) = %%\n", i, paths_.at(i).toUtf8().data());
				#endif
				Ref<Document> doc = request->document_;
				Ref<MatchStatus, Owner> status;
				
				if (replace) {
					View* view = request->edit_;
					if (doc->historyEnabled()) doc->begin(view->saveState());
					int cy = view->cursorLine();
					int cx = view->cursorColumn();
					status = doc->replace(pattern, substitute, &cy, &cx);
					view->moveCursor(cy, cx);
					if (doc->historyEnabled()) doc->end(view->saveState());
				}
				else {
					#if PACU_DEBUG_FINDTEXT
					debug("QxFindText::findAndReplace().2.1: paths_.at(%%) = %%\n", i, paths_.at(i).toUtf8().data());
					#endif
					status = doc->find(pattern);
					#if PACU_DEBUG_FINDTEXT
					debug("QxFindText::findAndReplace().2.2: paths_.at(%%) = %%\n", i, paths_.at(i).toUtf8().data());
					#endif
				}
				
				#if PACU_DEBUG_FINDTEXT
				debug("QxFindText::findAndReplace().3: paths_.at(%%) = %%\n", i, paths_.at(i).toUtf8().data());
				#endif
				
				if (status->matchingLines_->length() > 0) {
					status->matchingLines_->pushFront(
						new MatchingLine(doc, -1, 0)
					);
				}
				else {
					paths_.removeAt(i);
					--i;
					--n;
				}
				
				if (!matchStatus_) {
					matchStatus_ = status;
				}
				else {
					for (int i = 0, n = status->matchingLines_->length(); i < n; ++i)
						matchStatus_->matchingLines_->append(status->matchingLines_->get(i));
					if (matchStatus_->yMax_ < status->yMax_)
						matchStatus_->yMax_ = status->yMax_;
					matchStatus_->numFiles_ += (status->numMatches_ > 0);
					matchStatus_->numMatches_ += status->numMatches_;
				}
				
				#if PACU_DEBUG_FINDTEXT
				debug("QxFindText::findAndReplace().done\n");
				#endif
			}
			
			progress.setValue(progress.value() + 1);
		}
		
		if (progress.wasCanceled()) {
			dismiss();
			return;
		}
	}
	
	updateStatusMessage(true);
	
	replacement_->setEnabled((matchStatus_->matchingLines_->length() > 0) && (scope_->currentIndex() != Terminal));
	replaceButton_->setEnabled(replacement_->isEnabled());
	matchStatusDocument_ = Document::matchStatusDocument(matchStatus_);
	matchView_->setDocument(matchStatusDocument_);
	
	emit showFindStatusChanged(true);
}

void QxFindText::dismiss()
{
	matchStatus_ = 0;
	matchStatusDocument_ = 0;
	matchView_->setDocument(new Document(0));
	if (textView_) {
		if (textView_->hasSelection())
			textView_->unselect();
		textView_->update();
	}
	updateStatusMessage();
}

void QxFindText::updateStatusMessage(bool afterMatch)
{
	bool foundSomething = (matchStatus_) ? (matchStatus_->numMatches_ > 0) : false;
	if (foundSomething) {
		emit showMessage(
			tr("%1 match(es) found in %2 file(s)")
			.arg(matchStatus_->numMatches_)
			.arg(matchStatus_->numFiles_)
		);
	}
	else {
		if (afterMatch)
			emit showMessage(tr("Nothing found"));
		else
			emit showMessage(QString());
	}
}

void QxFindText::openLink()     { matchView_->openLink(); }
void QxFindText::firstLink()    { matchView_->firstLink(); }
void QxFindText::lastLink()     { matchView_->lastLink(); }
void QxFindText::previousLink() { matchView_->previousLink(); }
void QxFindText::nextLink()     { matchView_->nextLink(); }

void QxFindText::showEvent(QShowEvent* event)
{
	/*#ifdef QT_MAC_USE_COCOA
	if (textView_) { // prevent grabbing focus on application startup
		pattern_->setFocus();
		pattern_->lineEdit()->selectAll();
	}
	#endif*/
	emit showFindStatusChanged(true);
	updateStatusMessage(false);
	QxControl::showEvent(event);
}

void QxFindText::hideEvent(QHideEvent* event)
{
	emit showFindStatusChanged(false);
	emit showMessage(QString());
	// dismiss();
	QxControl::hideEvent(event);
}

} // namespace pacu
