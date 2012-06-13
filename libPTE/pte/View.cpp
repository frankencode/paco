#include <QDebug> // DEBUG
#include <ftl/streams> // DEBUG
#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTextLayout>
#include <QContextMenuEvent>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QScrollBar>
#include <QPrinter>
#include <QToolTip>
#include <ftl/utils>
#include "ScrollMachine.hpp"
#include "View.hpp"

#define PTE_VIEW_DEBUG_INPUTMETHOD 0
#define PTE_VIEW_RASTER_DISPLACEMENT -1

namespace pte
{

View::View(QWidget* parent, Ref<ViewMetrics> metrics, Ref<ViewColors> colors)
	: QWidget(parent),
	  exposedMetrics_(metrics),
	  colors_(colors),
	  renderCache_(new RenderCache),
	  layoutCache_(new LayoutCache),
	  cacheCapacity_(256),
	  annotationCache_(new AnnotationCache),
	  yScroll_(0),
	  leadingMargin_(0),
	  mouseCursorShape_(Qt::IBeamCursor),
	  preferTopToBottomRendering_(true),
	  showFindResult_(true),
	  commitLines_(false),
	  renderCaching_(false),
	  renderCachingMaxLineLength_(256),
	  printMode_(false),
	  wy_(256), wx_(4096),
	  gw_(0), gh_(0),
	  ga_(0),
	  yoli_(0),
	  yofir_(0),
	  yofia_(0),
	  nft_(0),
	  widthCounts_(new WidthCounts),
	  hasSelection_(false),
	  sely0_(-1), selx0_(-1),
	  sely1_(-1), selx1_(-1),
	  blinkTimer_(0),
	  blink_(false),
	  trx_(-1), try_(-1),
	  crx_(-1), cry_(-1),
	  crw_(-1), crh_(-1),
	  vya_(0), vxa_(0),
	  vyb_(0), vxb_(0),
	  tinyDocScrollRange_(false),
	  leftMouseButtonPressed_(false),
	  selectionAutoScroll_(false),
	  lineNumberOfActiveLink_(-1),
	  ylo_(0), yle_(intMax),
	  wheelFirstTime_(true),
	  wheelPreviousTime_(0),
	  scrollMachine_(new ScrollMachine(this))
{
	#ifdef Q_WS_X11
	setRenderCaching(true);
	#endif
	
	// performance HACKs
	/*if (renderCaching_)
		setAttribute(Qt::WA_NoSystemBackground, true); // seems to disable subpixel AA
	else*/
	setAttribute(Qt::WA_OpaquePaintEvent, true); // a little less effective than the above one
	
	setFocusPolicy(Qt::StrongFocus);
	setCursor(mouseCursorShape_);
	
	if (!exposedMetrics_)
		exposedMetrics_ = defaultMetrics();
	applyMetrics();
	
	if (!colors)
		colors_ = defaultColors();
	
	#ifdef QT_MAC_USE_COCOA
	{
		/*static const char* const xpm[] = {
			"8 16 3 1",
			"B c #000000",
			"W c #FFFFFF",
			". c None",
			"WBB..WBB",
			"..WBWB..",
			"...WB...",
			"...WB...",
			"...WB...",
			"...WB...",
			"...WB...",
			"...WB...",
			"..WBBB..",
			"...WB...",
			"...WB...",
			"...WB...",
			"...WB...",
			"...WB...",
			"..WBWB..",
			"WBB..WBB"
		};*/
		static const char* const xpm[] = {
			"7 16 4 1",
			"B c #000000",
			"W c #FFFFFF",
			"G c #BBBBBB",
			". c None",
			"BBBBBBB",
			"BWWGWWB",
			"BBBWBBB",
			"..BWB..",
			"..BWB..",
			"..BWB..",
			"..BWB..",
			"..BWB..",
			"..BWB..",
			"..BWB..",
			"..BWB..",
			"..BWB..",
			"..BWB..",
			"BBBWBBB",
			"BWWGWWB",
			"BBBBBBB"
		};
		ibeam_ = QPixmap(xpm);
	}
	#endif
	
	connect(scrollMachine_, SIGNAL(scroll(int)), this, SLOT(scroll(int)));
}

Ref<ViewMetrics, Owner> View::defaultMetrics() { return new ViewMetrics; }
Ref<ViewColors, Owner> View::defaultColors() { return new ViewColors; }

Ref<ViewColors> View::colors() const { return colors_; }
Ref<ViewMetrics> View::metrics() const { return exposedMetrics_; }

void View::applyMetrics()
{
	{
		int strategy = (exposedMetrics_->fontAntialiasing_) ? QFont::PreferOutline : QFont::PreferBitmap;
		strategy |= QFont::NoFontMerging /*although buggy on xorg...*/;
		// strategy |= QFont::PreferQuality;
		strategy |= QFont::PreferMatch; // most of the time faster on OSX!
		strategy |= (exposedMetrics_->fontAntialiasing_) ? QFont::PreferAntialias : QFont::NoAntialias;
		exposedMetrics_->font_.setStyleHint(QFont::TypeWriter, QFont::StyleStrategy(strategy));
	}
	#ifdef QT_MAC_USE_COCOA
	// exposedMetrics_->font_.setLetterSpacing(QFont::AbsoluteSpacing, 0);  // performance impact?
	#endif
	exposedMetrics_->font_.setKerning(false); // performance impact?
	exposedMetrics_->font_.setFixedPitch(true); // performance impact?
	
	QFontMetricsF fm(exposedMetrics_->font_);
	#ifdef Q_WS_MAC
		#ifndef QT_MAC_USE_COCOA
		gw_ = fm.averageCharWidth();
		#else
		gw_ = fm.width(QString(4096, ' ')) / 4096; // uggly HACK
		#endif
	#else
	gw_ = fm.width(QString(4096, ' ')) / 4096; // uggly HACK
	#endif
	#ifdef Q_WS_MAC
	gh_ = roundToInf(fm.ascent() + fm.descent() + exposedMetrics_->lineSpacing_);
	#else
	gh_ = roundToInf(fm.lineSpacing() + exposedMetrics_->lineSpacing_);
	#endif
	ga_ = roundToZero(fm.ascent());
	
	/*qDebug() << "fm.width(QString(4096, ' ')) / 4096 =" << fm.width(QString(4096, ' ')) / 4096.;
	qDebug() << "fm.averageCharWidth() =" << fm.averageCharWidth();*/
	
	// qDebug() << QString("fm.ascent(), fm.descent(), fm.leading() = %1, %2, %3").arg(fm.ascent()).arg(fm.descent()).arg(fm.leading());
	// qDebug() << "ga_, gh_, QFontInfo(exposedMetrics_->font_).pixelSize() =" << ga_ << "," << gh_ <<","<< QFontInfo(exposedMetrics_->font_).pixelSize();
	
	if ((exposedMetrics_->cursorStyle_ & BlinkingCursor) != 0)
	{
		if ((metrics_) ? ((metrics_->cursorStyle_ & BlinkingCursor) == 0) : true)
		{
			if (!blinkTimer_) {
				blinkTimer_ = new QTimer(this);
				blinkTimer_->setInterval(500);
				connect(blinkTimer_, SIGNAL(timeout()), this, SLOT(blink()), Qt::QueuedConnection);
			}
			if (hasFocus()) {
				blink_ = true;
				blinkTimer_->start();
			}
		}
	}
	else {
		if (blinkTimer_) {
			if (blinkTimer_->isActive())
				blinkTimer_->stop();
		}
	}
	
	if (document_)
		document_->setTabWidth(exposedMetrics_->tabWidth_);
	
	renderCaching_ = !exposedMetrics_->subpixelAntialiasing_;
	
	metrics_ = exposedMetrics_;
	
	cacheClear();
	
	if (hasSelection())
		unselect();
	
	metricsChanged();
	
	if (isVisible()) {
		QResizeEvent event(size(), size());
		QApplication::sendEvent(this, &event);
		update();
	}
}

Ref<Document> View::document() const {
	return document_;
}

void View::setDocument(Ref<Document> document)
{
	if (document_)
		disconnect(document_, SIGNAL(highlighterReady()), this, SLOT(update()));
	document_ = document;
	connect(document_, SIGNAL(highlighterReady()), this, SLOT(update()));
	document_->setTabWidth(metrics_->tabWidth_);
	document_->setDefaultStyle(new Style(colors_->foregroundColor_, colors_->backgroundColor_));
	if (hasSelection()) unselect();
	updateVerticalScrollRange();
	update();
}

Ref<Highlighter> View::highlighter() const {
	return document_->highlighter();
}

void View::setHighlighter(Ref<Highlighter> highlighter)
{
	document_->setHighlighter(highlighter);
	cacheClear();
	update();
}

QScrollBar* View::verticalScrollBar() const { return yScroll_; }

void View::setVerticalScrollBar(QScrollBar* scrollBar)
{
	yScroll_ = scrollBar;
	connect(yScroll_, SIGNAL(valueChanged(int)), this, SLOT(setYOrigin(int)));
}

bool View::showFindResult() const { return showFindResult_; }
void View::setShowFindResult(bool on) { showFindResult_ = on; update(); }

bool View::commitLines() const { return commitLines_; }
void View::setCommitLines(bool on) { commitLines_ = on; }

bool View::renderCaching() const { return renderCaching_; }
void View::setRenderCaching(bool on) { renderCaching_ = on; }

bool View::printMode() const { return printMode_; }
void View::setPrintMode(bool on) { printMode_ = on; }

int View::cursorLine() const { return intMax; }
int View::cursorColumn() const { return intMax; }
bool View::moveCursor(int cy, int cx) { moveCursorOverloaded_ = false; return false; }

void View::keepInView(int cy, int cx)
{
	if (cy == -1) cy = cursorLine();
	if (cx == -1) cx = cursorColumn();
	int h;
	int n = 1000;
	
	mapMouseToTextPos(0, 0, &h, &h);
	int d = cy - (vyb_ + vya_) / 2;
	if ((d < -n) || (n < d))
		setWindowOrigin(windowOrigin() + d);
	
	bool firstStep = true;
	bool stepBackwards;
	
	while (n > 0)
	{
		// qDebug() << QString("n: [cy, cx], [vya_, vyb_], windowOrigin() = %1: [%2,%3], [%4, %5], %6").arg(n).arg(cy).arg(cx).arg(vya_).arg(vyb_).arg(windowOrigin());
		if ((cy < vya_) || ((cy == vya_) && (cx < vxa_))) {
			if (!firstStep)
				if(!stepBackwards)
					break;
			// qDebug() << "back";
			stepBackwards = true;
			setWindowOrigin(windowOrigin() - 1);
		}
		else if ((cy > vyb_) || ((cy == vyb_) && (cx >= vxb_))) {
			if (!firstStep)
				if (stepBackwards)
					break;
			// qDebug() << "forward";
			stepBackwards = false;
			setWindowOrigin(windowOrigin() + 1);
		}
		else
			break;
		firstStep = false;
		mapMouseToTextPos(0, 0, &h, &h);
		--n;
	}
}

void View::blinkRestart()
{
	if (blinkTimer_) {
		if (blinkTimer_->isActive()) {
			blinkTimer_->start();
			blink_ = true;
		}
	}
}

bool View::hasBracketMatch() const { return false; }
void View::getBracketMatch(int* bmy0, int* bmx0, int* bmy1, int* bmx1) {}

int View::windowLines() const { return wy_; }
int View::windowColumns() const { return wx_; }
int View::windowOrigin() const { return verticalScrollBar() ? verticalScrollBar()->value() : yoli_ /*intermediate HACk, should be yofia_*/; }
void View::setWindowOrigin(int value) {
	if (verticalScrollBar()) verticalScrollBar()->setValue(value);
	else yoli_ = value; // intermediate HACK
	// else followScrollPos(value);
}

void View::setWindowSizeHint(int ny, int nx)
{
	sizeHint_ = QSize(nx * gw_ + 2 * metrics_->blanking_, ny * gh_ + 2 * metrics_->blanking_);
}

QSize View::sizeHint() const { return sizeHint_; }

void View::getViewRange(int* vya, int* vxa, int* vyb, int* vxb, bool* tinyDoc)
{
	*vya = vya_;
	*vxa = vxa_;
	*vyb = vyb_;
	*vxb = vxb_;
	if (tinyDoc) *tinyDoc = tinyDocScrollRange_;
}

QString View::saveState()
{
	QString state;
	QTextStream out(&state);
	out << cursorLine() << ' ' << cursorColumn() << ' ';
	out << int(hasSelection_) << ' ';
	if (hasSelection_) {
		int selya, selxa, selyb, selxb;
		getSelection(&selya, &selxa, &selyb, &selxb);
		out << selya << ' ' << selxa << ' ' << selyb << ' ' << selxb << ' ';
	}
	else {
		out << "-1 -1 -1 -1 ";
	}
	out << windowOrigin();
	return state;
}

void View::restoreState(QString state)
{
	QTextStream in(&state);
	int cy, cx;
	in >> cy >> cx;
	moveCursor(cy, cx);
	int h;
	in >> h;
	setHasSelection(h);
	in >> sely0_ >> selx0_ >> sely1_ >> selx1_;
	int yo;
	in >> yo;
	setWindowOrigin(yo);
	keepInView();
}

void View::setColors(Ref<ViewColors> colors)
{
	if (!colors) return;
	colors_ = colors;
	if (document_)
		document_->setDefaultStyle(new Style(colors_->foregroundColor_, colors_->backgroundColor_));
	cacheClear();
	if (hasSelection())
		unselect();
	if (isVisible()) {
		
		update();
	}
}

void View::setMetrics(Ref<ViewMetrics> metrics)
{
	exposedMetrics_ = metrics;
	applyMetrics();
}

void View::gotoLine(int y)
{
	if ((0 <= y) && (y < cache()->length())) {
		highlighterYield();
		moveCursor(y, 0);
		setWindowOrigin(y - 1);
		highlighterResume();
		update();
	}
}

void View::openLink()
{
	if (activeLink()) {
		highlighterYield();
		emit gotoLinkRequest(activeLink());
		highlighterResume();
	}
}

void View::firstLink()
{
	highlighterYield();
	for (int y = ylo_, n = cache()->length(); (y < yle_) && (y < n); ++y)
	{
		Ref<LinkList> linkList = cache()->get(y)->linkList_;
		if (linkList) {
			activeLink_ = linkList->get(0);
			lineNumberOfActiveLink_ = y;
			keepInView(y, 0);
			repaint();
			break;
		}
	}
	highlighterResume();
}

void View::lastLink()
{
	highlighterYield();
	for (int n = cache()->length(), y = ((n < yle_) ? n : yle_) - 1; y >= ylo_; --y)
	{
		Ref<LinkList> linkList = cache()->get(y)->linkList_;
		if (linkList) {
			activeLink_ = linkList->get(linkList->length() - 1);
			lineNumberOfActiveLink_ = y;
			keepInView(y, 0);
			repaint();
			break;
		}
	}
	highlighterResume();
}

void View::previousLink()
{
	if (!activeLink_) { lastLink(); return; }
	
	int y = lineNumberOfActiveLink_;
	if ((!activeLink_) || (y < ylo_) || (cache()->length() <= y)) return;
	
	Ref<LinkList> linkList = cache()->get(y)->linkList_;
	if (!linkList) return;
	
	int i = linkList->find(activeLink_);
	if (i > 0) {
		activeLink_ = linkList->get(i - 1);
		repaint();
		return;
	}
	
	highlighterYield();
	
	while (y > ylo_) {
		--y;
		linkList = cache()->get(y)->linkList_;
		if (linkList) {
			if (linkList->length() > 0) {
				activeLink_ = linkList->get(linkList->length() - 1);
				lineNumberOfActiveLink_ = y;
				keepInView(y, 0);
				repaint();
				break;
			}
		}
	}
	
	highlighterResume();
}

void View::nextLink()
{
	if (!activeLink_) { firstLink(); return; }
	
	Ref<Link> link = activeLink_;
	int y = lineNumberOfActiveLink_;
	if ((y < ylo_) || (cache()->length() <= y)) return;
	
	Ref<LinkList> linkList = cache()->get(y)->linkList_;
	if (!linkList) return;
	
	int i = linkList->find(activeLink_);
	if ((0 <= i) && (i < linkList->length() - 1)) {
		activeLink_ = linkList->get(i + 1);
		repaint();
		return;
	}
	
	highlighterYield();
	
	int ye = (cache()->length() < yle_) ? cache()->length() : yle_;
	while (y < ye) {
		++y;
		linkList = cache()->get(y)->linkList_;
		if (linkList) {
			if (linkList->length() > 0) {
				activeLink_ = linkList->get(0);
				lineNumberOfActiveLink_ = y;
				keepInView(y, 0);
				repaint();
				break;
			}
		}
	}
	
	highlighterResume();
}

void View::selectionAutoScroll()
{
	selectionAutoScroll_ = false;
	if (hasSelection() && leftMouseButtonPressed_) {
		int wy0 = mapToGlobal(QPoint(0, 0)).y();
		int wy1 = wy0 + height();
		if (QCursor::pos().y() < wy0) {
			--sely1_;
			selectionAutoScroll_ = true;
		}
		else if (QCursor::pos().y() >= wy1) {
			++sely1_;
			selectionAutoScroll_ = true;
		}
		if (selectionAutoScroll_) {
			document()->stepNearest(&sely1_, &selx1_);
			moveCursor(sely1_, selx1_);
			keepInView(sely1_, selx1_);
			blinkRestart();
			setHasSelection((sely0_ != sely1_) || (selx0_ != selx1_));
			update();
			QTimer::singleShot(metrics_->selectionAutoScrollTimeout_, this, SLOT(selectionAutoScroll()));
		}
	}
}

void View::fitToPage(QSize page)
{
	resize(page.width(), ((page.height() - 2 * metrics_->blanking_) / gh_) * gh_ + 2 * metrics_->blanking_);
}

bool View::drawPage(QPainter* p)
{
	if (p) p->save();
	renderTo(p);
	if (p) p->restore();
	int vya, vxa, vyb, vxb;
	getViewRange(&vya, &vxa, &vyb, &vxb);
	if (vyb >= windowOrigin())
		setWindowOrigin(vyb + 1);
	else
		setWindowOrigin(windowOrigin() + 1); // partial workaround HACK for very long lines
	// qDebug() << "vyb =" << vyb;
	return vyb + 1 < cache()->length();
}

void View::print(QPrinter* printer, Ref<ViewMetrics> metrics, bool pageHeader, bool pageBorder)
{
	highlighterYield();
	highlighterSync();
	
	View* printView = new View(this);
	printView->setRenderCaching(false);
	printView->setPrintMode(true);
	printView->setDocument(document());
	printView->setColors(colors());
	printView->setMetrics(metrics);
	QRect page = printer->pageRect();
	QFontMetrics fm(printView->metrics()->font_);
	int b = printView->metrics()->blanking_;
	QRect header(b, 0, page.width() - 2 * b, fm.ascent() + fm.descent() + fm.leading() + 2 * b);
	page.setHeight(page.height() - header.height());
	printView->fitToPage(page.size());
	QString fileName = QFileInfo(printView->document()->filePath()).fileName();
	int numPages = 1;
	if (pageHeader) {
		while (printView->drawPage(0))
			++numPages;
		printView->setWindowOrigin(0);
	}
	
	int pageCount = 0;
	QPainter painter;
	painter.begin(printer);
	while (true) {
		if (pageHeader) {
			painter.setFont(printView->metrics()->font_);
			painter.setPen(printView->colors_->lineNumbersFgColor_);
			painter.drawText(header, Qt::AlignVCenter|Qt::AlignLeft, fileName);
			++pageCount;
			painter.drawText(header, Qt::AlignVCenter|Qt::AlignRight, QString("%1/%2").arg(pageCount).arg(numPages));
			painter.save();
			painter.translate(0, header.height());
		}
		bool eod = !printView->drawPage(&painter);
		if (pageBorder) {
			painter.setPen(printView->colors_->lineNumbersBgColor_);
			painter.drawRect(0, 0, printView->width(), printView->height());
		}
		if (pageHeader)
			painter.restore();
		if (eod)
			break;
		printer->newPage();
	}
	painter.end();
	delete printView;
	
	highlighterResume();
}

QString View::css(Style* style, bool noBg)
{
	QStringList sl;
	sl << "color:" << style->fgColor_.name() << ";";
	if ((!noBg) && (style->bgColor_ != Qt::transparent))
		sl << "background-color:" << style->bgColor_.name() << ";";
	sl << "font-weight:" << (style->bold_ ? "bold" : "normal") << ";";
	sl << "text-decoration:" << (style->underline_ ? "underline" : "none") << ";";
	return sl.join("");
}

QString View::cssNoBg(Style* style) { return View::css(style, true); }

void View::exportHtml(QTextStream* textSink)
{
	highlighterYield();
	highlighterSync();
	
	*textSink << "<pre style=\"" << css(document_->defaultStyle()) << "\">\n";
	for (int y = 0; y < document_->cache()->length(); ++y)
	{
		Ref<Line> line = document_->cache()->get(y);
		document_->updateTabWidths(y);
		Ref<ChunkList> chunkList = line->chunkList_;
		for (int i = 0; i < chunkList->length(); ++i) {
			Ref<Chunk> chunk = chunkList->get(i);
			QString text = line->replaceTabBySpace(chunk->text_, chunk->x0_);
			text.replace("&", "&amp;");
			text.replace("<", "&lt;");
			text.replace(">", "&gt;");
			*textSink << "<span style=\"" << cssNoBg(chunk->style_) << "\">" << text << "</span>";
		}
		*textSink << "\n";
	}
	*textSink << "</pre>\n";
	
	highlighterResume();
}

QMenu* View::contextMenu() const { return contextMenu_; }

void View::setContextMenu(QMenu* contextMenu) {
	/*addActions(contextMenu->actions());
	QList<QAction*> actions = contextMenu()->actions();
	for (int i = 0, n = actions.length(); i < n; ++i) {
		
	}*/
	contextMenu_ = contextMenu;
}

QVariant View::inputMethodQuery(Qt::InputMethodQuery query) const
{
	#if PTE_VIEW_DEBUG_INPUTMETHOD
	QString queryName = "<Unknown query type>";
	#endif
	QVariant reply;
	switch (query) {
		case Qt::ImMicroFocus:
			#if PTE_VIEW_DEBUG_INPUTMETHOD
			queryName = "Qt::ImMicroFocus";
			#endif
			reply = QRect(crx_, cry_, crw_, crh_);
			break;
		case Qt::ImFont:
			#if PTE_VIEW_DEBUG_INPUTMETHOD
			queryName = "Qt::ImFont";
			#endif
			reply = metrics_->font_;
			break;
		case Qt::ImCursorPosition:
			#if PTE_VIEW_DEBUG_INPUTMETHOD
			queryName = "Qt::ImCursorPosition";
			#endif
			reply = cursorColumn();
			break;
		case Qt::ImSurroundingText:
			#if PTE_VIEW_DEBUG_INPUTMETHOD
			queryName = "Qt::ImSurroundingText";
			#endif
			reply = document()->copySpan(cursorLine(), 0, intMax, ' ', true);
			break;
		case Qt::ImCurrentSelection:
			#if PTE_VIEW_DEBUG_INPUTMETHOD
			queryName = "Qt::ImCurrentSelection";
			#endif
			reply = copySelection();
			break;
		case Qt::ImMaximumTextLength:
			#if PTE_VIEW_DEBUG_INPUTMETHOD
			queryName = "Qt::ImMaximumTextLength";
			#endif
			reply = QVariant(); // no limit
			break;
		case Qt::ImAnchorPosition: {
			#if PTE_VIEW_DEBUG_INPUTMETHOD
			queryName = "Qt::ImAnchorPosition";
			#endif
			int cy = cursorLine();
			QString line = document()->copySpan(cy, 0, intMax, ' ', true);
			int selya, selxa, selyb, selxb;
			getSelection(&selya, &selxa, &selyb, &selxb);
			if (selya < cy) {
				selya = cy;
				selxa = 0;
			}
			if (selya > cy) {
				selya = cy;
				selxa = line.length();
			}
			reply = selxa;
			break;
		}
		default:
			reply = QVariant();
	}
	
	#if PTE_VIEW_DEBUG_INPUTMETHOD
	qDebug() << "View::inputMethodQuery(): queryName, reply = " << queryName << "," << reply;
	#endif
	
	return reply;
}

Ref<ViewMetrics> View::activeMetrics() const { return metrics_; }
void View::metricsChanged() {}

Ref<Cache> View::cache() const { return document_->cache(); }
// Ref<View::RenderCache> View::renderCache() const { return renderCache_; }
int View::renderCachingMaxLineLength() const { return renderCachingMaxLineLength_; }

void View::cacheClear()
{
	renderCache_->clear();
	layoutCache_->clear();
	annotationCache_->clear();
}

bool View::leadingMargin() const { return leadingMargin_; }
void View::setLeadingMargin(int numChars) { leadingMargin_ = numChars; }

Qt::CursorShape View::mouseCursorShape() const { return mouseCursorShape_; }
void View::setMouseCursorShape(Qt::CursorShape shape) { mouseCursorShape_ = shape; setCursor(shape); }

bool View::preferTopToBottomRendering() const { return preferTopToBottomRendering_; }
void View::setPreferTopToBottomRendering(bool on) { preferTopToBottomRendering_ = on; }

bool View::tinyDocScrollRange() const { return tinyDocScrollRange_; }

void View::updateVerticalScrollRange()
{
	if (yScroll_) {
		int yoMax = cache()->length() - wy_;
		if (yoMax <= 0) yoMax = int(tinyDocScrollRange_);
		if (yoMax != yScroll_->maximum())
			yScroll_->setMaximum(yoMax);
	}
}

int View::glyphWidth() const { return gw_; }
int View::glyphHeight() const { return gh_; }
int View::glyphAscent() const { return ga_; }

void View::mapMouseToTextPos(int mx, int my, int* ty, int* tx, bool mapExact)
{
	my_ = my - metrics_->blanking_;
	mx_ = mx - metrics_->blanking_;
	ty_ = -1;
	tx_ = -1;
	mapExact_ = mapExact;
	// Time t0 = now();
	renderTo();
	// ftl::print("empty render: dt = %% us\n", (now() - t0).us());
	*ty = ty_;
	*tx = tx_;
}

void View::updateAfterLineModified(int y)
{
	Ref<Line> line = document_->cache()->get(y);
	if ((numFragments(line) == line->numFragments_) && (!hasBracketMatch())) {
		if (line->layout_) {
			int nf = numFragments(line);
			if (nf == 0) nf = 1;
			QFontMetricsF fm(metrics_->font_);
			QRect rect(0, line->yr_ + metrics_->lineSpacing_ - 1, width(), gh_ * nf + 2); // -1/+2 empirical HACK
			update(rect);
			return;
		}
	}
	update();
}

void View::updateAfterCursorMoved(int ya, int yb)
{
	if (ya > yb) {
		int h = yb;
		yb = ya;
		ya = h;
	}
	if ((vya_ <= ya) && (yb <= vyb_) && (!hasBracketMatch())) {
		Ref<Line> la = document_->cache()->get(ya);
		Ref<Line> lb = document_->cache()->get(yb);
		int nl = numFragments(lb);
		if (nl == 0) nl = 1;
		QRect rect(0, la->yr_ - 1, width(), (lb->yr_ - la->yr_) + gh_ * nl + 2); // -1/+2 empirical HACK
		update(rect);
		return;
	}
	update();
}

bool View::hasSelection() const { return hasSelection_; }

/** get sorted selection coords:
  *     (selya, selxa)#########
  * #############################
  * ########(selyb, selxb)
  **/
void View::getSelection(int* selya, int* selxa, int* selyb, int* selxb) const
{
	*selya = -1; *selxa = -1;
	*selyb = -1; *selxb = -1;
	if (hasSelection_) {
		if (sely0_ == sely1_) {
			if (selx0_ < selx1_) {
				if (selya) *selya = sely0_;
				if (selxa) *selxa = selx0_;
				if (selyb) *selyb = sely1_;
				if (selxb) *selxb = selx1_;
			} else {
				if (selya) *selya = sely1_;
				if (selxa) *selxa = selx1_;
				if (selyb) *selyb = sely0_;
				if (selxb) *selxb = selx0_;
			}
		}
		else {
			if (sely0_ < sely1_) {
				if (selya) *selya = sely0_;
				if (selxa) *selxa = selx0_;
				if (selyb) *selyb = sely1_;
				if (selxb) *selxb = selx1_;
			} else {
				if (selya) *selya = sely1_;
				if (selxa) *selxa = selx1_;
				if (selyb) *selyb = sely0_;
				if (selxb) *selxb = selx0_;
			}
		}
	}
}

void View::select(int sely0, int selx0, int sely1, int selx1)
{
	sely0_ = sely0;
	selx0_ = selx0;
	sely1_ = sely1;
	selx1_ = selx1;
	setHasSelection(true);
}

void View::selectAll()
{
	sely0_ = 0;
	selx0_ = 0;
	sely1_ = cache()->length() - 1;
	selx1_ = 0;
	Ref<ChunkList> chunkList = cache()->get(sely1_)->chunkList_;
	if (chunkList->length() > 0)
		selx1_ = chunkList->get(-1)->x1_;
	setHasSelection(true);
}

void View::unselect() { setHasSelection(false); }

void View::startSelection(int sely0, int selx0)
{
	setHasSelection(false);
	sely0_ = sely0;
	selx0_ = selx0;
}

void View::extendSelection(int sely1, int selx1)
{
	sely1_ = sely1;
	selx1_ = selx1;
	setHasSelection(((sely0_ != sely1_) || (selx0_ != selx1_)));
}

void View::deleteSelection()
{
	if (hasSelection()) {
		int selya, selxa, selyb, selxb;
		getSelection(&selya, &selxa, &selyb, &selxb);
		unselect();
		document()->pop(selya, selxa, selyb, selxb);
		moveCursor(selya, selxa);
	}
}

bool View::expandToWord(int ty, int tx, int* x0, int* x1) const
{
	QString s = document()->getChar(ty, tx);
	bool canExpand = isWord(s[0]);
	if (canExpand) {
		*x0 = tx;
		*x1 = tx + 1;
		while (true) {
			s = document()->getChar(ty, *x0 - 1);
			if (!isWord(s[0])) break;
			--*x0;
		}
		while (true) {
			s = document()->getChar(ty, *x1);
			if (!isWord(s[0])) break;
			++*x1;
		}
	}
	return canExpand;
}

QString View::copySelection() const
{
	if (!hasSelection())
		return QString();
	int selya, selxa;
	int selyb, selxb;
	getSelection(&selya, &selxa, &selyb, &selxb);
	return document()->copy(selya, selxa, selyb, selxb);
}

bool View::isWord(QChar ch) const
{
	return (('a' <= ch) && (ch <= 'z')) ||
	       (('A' <= ch) && (ch <= 'Z')) ||
	       (('0' <= ch) && (ch <= '9')) ||
	       (ch == '_');
}

QString View::wordUnderCursor() const
{
	QString word;
	int ty = cursorLine();
	int tx = cursorColumn();
	int x0, x1;
	if (expandToWord(ty, tx, &x0, &x1))
		word = document()->copySpan(ty, x0, x1);
	return word;
}

void View::commitLine(int y)
{
	if (commitLines_) {
		Ref<LinkList> linkList = cache()->get(y)->linkList_;
		if (linkList) linkList->clear();
		emit lineCommitted(y);
	}
}

Ref<Link> View::activeLink() const { return activeLink_; }

void View::setLinkRange(int ylo, int yle)
{
	ylo_ = ylo;
	yle_ = yle;
}

void View::resetLinkRange()
{
	ylo_ = 0;
	yle_ = intMax;
}

int View::linksInRange(int ylo, int yle) const
{
	int m = 0;
	for (int y = ylo, ye = (cache()->length() < yle) ? cache()->length() : yle; y < ye; ++y) {
		Ref<LinkList> linkList = cache()->get(y)->linkList_;
		if (linkList) m += linkList->length();
	}
	return m;
}

void View::showTip(const QString& text) { QToolTip::showText(mapToGlobal(QPoint(crx_, cry_)), text, this); }
void View::hideTip() { if (QToolTip::isVisible()) QToolTip::hideText(); }
QPoint View::cursorRenderPos() { return QPoint(crx_, cry_ + gh_); }

void View::paintEvent(QPaintEvent* event)
{
	// Time t0 = now();
	highlighterYield();
	
	// Time t1 = now();
	QPainter p(this);
	if (!event->region().isEmpty())
		p.setClipRegion(event->region());
	renderTo(&p);
	
	#if 0
	if (!bgBufferPassThrough_) {
		if (!event->region().isEmpty()) {
			// qDebug() << "View::paintEvent(): event->region() =" << event->region();
			p.setClipRegion(event->region());
		}
		// Time t2 = now();
		//renderTo(&p);
		bool resizeBgBuffer = (bgBuffer_.isNull()) ? true : ((bgBuffer_.width() != width()) || (bgBuffer_.height() != height()));
		if (resizeBgBuffer) {
			QPixmap oldBuffer = bgBuffer_;
			bgBuffer_ = QPixmap(width(), height());
			if (!oldBuffer.isNull())
				QPainter(&bgBuffer_).drawPixmap(0, 0, oldBuffer);
		}
		{
			bgBuffer_.fill(defaultStyle()->bgColor_);
			QPainter p(&bgBuffer_);
			if (!event->region().isEmpty())
				p.setClipRegion(event->region());
			renderTo(&p);
		}
	}
	else
		bgBufferPassThrough_ = false;
	p.drawPixmap(0, 0, bgBuffer_);
	#endif
	// ftl::print("View::paintEvent(): dt0, dt1, dt2 = %% ms, %% ms, %% ms\n", (t1 - t0).ms(), (t2 - t1).ms(), (now() - t2).ms());
	// ftl::print("View::paintEvent(): dt0 = %%\n", (now() - t0).ms());
	
	highlighterResume();
}


void View::resizeEvent(QResizeEvent* event)
{
	wy_ = (height() - 2 * metrics_->blanking_) / gh_;
	wx_ = (width() - 2 * metrics_->blanking_) / gw_;
	
	if (wy_ < 0) wy_ = 0;
	if (wx_ < 0) wx_ = 0;
	
	// resize caches
	cacheCapacity_ = wy_ * 2;
	while (renderCache_->length() > wy_ * 2)
		renderCache_->popFront();
	while (layoutCache_->length() > wy_ * 2)
		layoutCache_->popFront();
	
	if (metrics_->showLineNumbers_) {
		int nl = lineNumbersNumDigits() + 2;
		wx_ -= nl;
	}
	else
		wx_ -= leadingMargin_;
	if (wx_ < 0) wx_ = 0;
	
	updateVerticalScrollRange();
}

void View::focusInEvent(QFocusEvent* event)
{
	if (blinkTimer_)
		blinkTimer_->start();
	blink_ = true;
	update();
}

void View::focusOutEvent(QFocusEvent* event)
{
	if (blinkTimer_)
		blinkTimer_->stop();
	blink_ = false;
	update();
}

void View::hideEvent(QHideEvent* event)
{
	cacheClear();
	if (!bgBuffer_.isNull())
		bgBuffer_ = QPixmap();
}

bool View::event(QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		// prevent tab key to change focus
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);
		if ( (ke->key() == Qt::Key_Tab) ||
		     (ke->key() == Qt::Key_Backtab) ) {
			keyPressEvent(ke);
			ke->accept();
			return true;
		}
	}
	#ifdef Q_WS_MAC
	else if (event->type() == QEvent::ShortcutOverride)
	{
		// workaround HACK for QAction::shortcutContext() == Qt::WidgetContext not working properly (on OSX)
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);
		QKeySequence seq(ke->modifiers() + ke->key());
		QList<QAction*> l = actions();
		for (int i = 0, n = l.count(); i < n; ++i) {
			QAction* action = l.at(i);
			if (action->isEnabled() && (action->shortcut() != QKeySequence()) && action->shortcut().matches(seq)) {
				if (!event->spontaneous())
					action->activate(QAction::Trigger);
				ke->accept();
				return true;
			}
		}
	}
	#endif
	return QWidget::event(event);
}

void View::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		highlighterYield();
		
		if (activeLink_) {
			emit gotoLinkRequest(activeLink_);
		}
		else {
			int ty, tx;
			mapMouseToTextPos(event->x() + glyphWidth() / 2, event->y(), &ty, &tx);
			
			int cy = cursorLine(), cx = cursorColumn();
			
			if (event->modifiers() & Qt::ShiftModifier)
			{
				if ((ty != cy) || (tx != cx)) {
					if (!hasSelection())
						startSelection(cy, cx);
					extendSelection(ty, tx);
					moveCursor(ty, tx);
					blinkRestart();
					update();
				}
			}
			else {
				if ((ty != -1) && (tx != -1)) {
					moveCursor(ty, tx);
					startSelection(ty, tx);
					blinkRestart();
					update();
				}
				else if (hasSelection()) {
					unselect();
					update();
				}
			}
			
			leftMouseButtonPressed_ = true;
		}
		
		highlighterResume();
	}
}

void View::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		leftMouseButtonPressed_ = false;
}

void View::mouseMoveEvent(QMouseEvent* event)
{
	highlighterYield();
	
	watchLinkBelowCursor(event->x(), event->y());
	if (leftMouseButtonPressed_)
	{
		int ty, tx;
		mapMouseToTextPos(event->x() + glyphWidth() / 2, event->y(), &ty, &tx);
		if ((ty != -1) && (tx != -1)) {
			int cy2 = cursorLine();
			moveCursorOverloaded_ = true;
			if (moveCursor(ty, tx))
				keepInView();
			else if (!moveCursorOverloaded_)
				cy2 = sely1_;
			blinkRestart();
			sely1_ = ty;
			selx1_ = tx;
			setHasSelection(((sely0_ != sely1_) || (selx0_ != selx1_)) || hasSelection()); // HACK, allowing empty selection?
			// setHasSelection(true);
			updateAfterCursorMoved(ty, cy2);
		}
		else if (!selectionAutoScroll_) {
			int y = event->pos().y();
			if ((y < 0) || (height() <= y)) {
				selectionAutoScroll_ = true;
				QTimer::singleShot(metrics_->selectionAutoScrollTimeout_, this, SLOT(selectionAutoScroll()));
			}
		}
	}
	/*#ifdef Q_WS_MAC
	#ifdef QT_MAC_USE_COCOA
	// workaround HACK for broken separators (aka splitters) in QMainWindow
	if (event->x() < widthOfLineNumbersColumn() * gw_)
		unsetCursor();
	else if (!activeLink_)
		setCursor(mouseCursorShape());
	#endif // QT_MAC_USE_COCOA
	#endif // QT_MAC_USE_COCOA*/
	
	highlighterResume();
}

#ifdef QT_MAC_USE_COCOA
// workaround HACK, fixes broken cursor handling
void View::enterEvent(QEvent* event)
{
	updateCursor();
}
#endif

void View::leaveEvent(QEvent* event)
{
	highlighterYield();
	
	watchLinkBelowCursor(-1, -1);
	
	highlighterResume();
}

void View::mouseDoubleClickEvent(QMouseEvent* event)
{
	highlighterYield();
	
	int ty, tx;
	mapMouseToTextPos(event->pos().x(), event->pos().y(), &ty, &tx);
	if (ty != -1) {
		if (expandToWord(ty, tx, &selx0_, &selx1_)) {
			setHasSelection(true);
			sely0_ = ty;
			sely1_ = ty;
			update();
		}
		else { // select whole line
			setHasSelection(true);
			sely0_ = ty;
			sely1_ = ty;
			selx0_ = 0;
			selx1_ = intMax;
			update();
		}
	}
	leftMouseButtonPressed_ = false;
	
	highlighterResume();
}

int View::stepsFromWheelEvent(QWheelEvent* event)
{
	const int granularity = 8 * 15; // assume steps in multiple of 15 degrees, each 3 bit
	return (event->delta() / granularity) * metrics_->scrollingSpeed_;
}

void View::wheelEvent(QWheelEvent* event)
{
	// qDebug() << "View::wheelEvent(): event->delta() / 8. / 15. =" << event->delta() / 8. / 15.;
	if (yScroll_) {
		if (metrics_->kinematicScrolling_) {
			scrollMachine_->wheel(event);
		}
		else {
			QCoreApplication::sendEvent(yScroll_, event);
		}
	}
}

void View::scroll(int steps)
{
	// qDebug() << "steps =" << steps;
	highlighterYield();
	yScroll_->setValue(yScroll_->value() - steps);
	highlighterResume();
}

void View::contextMenuEvent(QContextMenuEvent* event)
{
	if (contextMenu_)
		contextMenu_->exec(event->globalPos());
}

void View::setYOrigin(int yo)
{
	update();
}

void View::blink()
{
	blink_ = !blink_;
	if (crx_ != -1) {
		update(crx_, cry_, crw_, crh_);
	}
	else {
		update(); // HACK
		// ftl::print("View::blink(): fullscreen update\n");
	}
}

void View::updateTotalNumberOfFragmentsBeforeModified(Ref<Delta> delta)
{
	if ((delta->type_ == Delta::PushSpan) || (delta->type_ == Delta::PopSpan)) {
		Ref<Line> line = document_->cache()->get(delta->y_);
		int nc = line->width();
		nft_ -= numFragmentsFromWidth(nc);
		--(*widthCounts_)[nc];
	}
}

void View::updateTotalNumberOfFragmentsAfterModified(Ref<Delta> delta)
{
	if ((delta->type_ == Delta::PushSpan) || (delta->type_ == Delta::PopSpan)) {
		Ref<Line> line = document_->cache()->get(delta->y_);
		int nc = line->width();
		nft_ += numFragmentsFromWidth(nc);
		++(*widthCounts_)[nc];
	}
	else if (delta->type_ == Delta::PushLine) {
		++nft_;
		++(*widthCounts_)[0];
	}
	else if (delta->type_ == Delta::PopLine) {
		--nft_;
		--(*widthCounts_)[0];
	}
	if (yScroll_) {
		if (yScroll_->maximum() != nft_)
			yScroll_->setMaximum(nft_);
	}
}

void View::initializeTotalNumberOfFragments()
{
	nft_ = 0;
	widthCounts_->clear();
	for (int y = 0, n = document_->cache()->length(); y < n; ++y) {
		Ref<Line> line = document_->cache()->get(y);
		int nc = line->width();
		int nf = numFragmentsFromWidth(nc);
		++(*widthCounts_)[nc];
		nft_ += nf;
	}
	if (yScroll_) {
		if (yScroll_->maximum() != nft_)
			yScroll_->setMaximum(nft_);
	}
}

void View::updateTotalNumberOfFragments()
{
	nft_ = 0;
	for (int i = 0; i < widthCounts_->length(); ++i) {
		WidthCounts::Item item = widthCounts_->get(i);
		int nc = item.key(), ncc = item.value();
		int nf = numFragmentsFromWidth(nc);
		nft_ += nf * ncc;
	}
	if (yScroll_) {
		if (yScroll_->maximum() != nft_)
			yScroll_->setMaximum(nft_);
	}
}

void View::followScrollPos(int yofia2)
{
	{
		while ((yofia_ > yofia2) && (yofia_ > 0))
		{
			if (yofir_ == 0) {
				if (yoli_ == 0)
					break;
				--yoli_;
				yofir_ = numFragments(yoli_) - 1;
			}
			else
				--yofir_;
			--yofia_;
		}
	}
	{
		int nf = numFragments(yoli_);
		while ((yofia_ < yofia2) && (yofia_ < nft_))
		{
			if (yofir_ == nf - 1) {
				if (yoli_ == nft_ - 1)
					break;
				++yoli_;
				nf = numFragments(yoli_);
				yofir_ = 0;
			}
			else
				++yofir_;
			++yofia_;
		}
	}
}

Ref<Link> View::linkBelowCursor(int mx, int my, int* y)
{
	Ref<Link> link;
	int ty = -1, tx = -1;
	if ((my > 0) && (mx > 0))
		mapMouseToTextPos(mx, my, &ty, &tx, true);
	if ((ty != -1) && (tx != -1)) {
		Ref<Line> line = cache()->get(ty);
		Ref<LinkList> linkList = line->linkList_;
		if (linkList) {
			for (int i = 0, n = linkList->length(); i < n; ++i) {
				Ref<Link> candidate = linkList->get(i);
				if ((candidate->x0_ <= tx) && (tx <= candidate->x1_)) {
					link = candidate;
					break;
				}
			}
		}
	}
	if (y) *y = ty;
	return link;
}

void View::watchLinkBelowCursor(int mx, int my)
{
	Ref<Link> link = linkBelowCursor(mx, my, &lineNumberOfActiveLink_);
	if (link != activeLink_) {
		if (link) {
			setCursor(Qt::PointingHandCursor);
		}
		else {
			#ifdef QT_MAC_USE_COCOA
			updateCursor();
			#else
			setCursor(mouseCursorShape());
			#endif
		}
		activeLink_ = link;
		repaint();
	}
}

#ifdef QT_MAC_USE_COCOA
void View::updateCursor()
{
	if ((mouseCursorShape_ == Qt::IBeamCursor) && (colors_->backgroundColor_.lightnessF() < 0.5))
		setCursor(ibeam_);
	else
		setCursor(mouseCursorShape_);
}
#endif


Ref<Style> View::defaultStyle() const { return document_->defaultStyle(); }

void View::setHasSelection(bool value)
{
	bool h = hasSelection_;
	hasSelection_ = value;
	if (h != value)
		emit hasSelectionChanged(value);
}

int View::numDigits(int x, int b) const
{
	int n = 1;
	int y = b;
	while (y <= x) {
		y *= b;
		++n;
	}
	return n;
}

int View::lineNumbersNumDigits() const
{
	Ref<MatchStatus> matchStatus = document_->matchStatus();
	int yMax = (matchStatus) ? matchStatus->yMax_ : cache()->length();
	return (cache()->length() > 0) ? numDigits(yMax) : 1;
}

int View::widthOfLineNumbersColumn() const
{
	return (metrics_->showLineNumbers_) ? lineNumbersNumDigits() + 2 : leadingMargin_;
}

int View::numFragmentsFromWidth(int nc)
{
	return nc / wx_ + (nc % wx_ > 0);
}

int View::numFragments(int y)
{
	Ref<Line> line = document_->cache()->get(y);
	return numFragmentsFromWidth(line->width());
}

int View::numFragments(Ref<Line> line)
{
	return numFragmentsFromWidth(line->width());
}

int View::firstTrailingSpace(Ref<Line> line)
{
	Ref<ChunkList> chunkList = line->chunkList_;
	int xts = 0;
	for (int k = 0; k < chunkList->length(); ++k)
	{
		Ref<Chunk> chunk = chunkList->get(k);
		int x = chunk->x0_;
		for (int j = 0; j < chunk->text_.length(); ++j)
		{
			QChar ch = chunk->text_.at(j);
			if (ch == '\t') x += line->tabWidth(chunk->x0_ + j);
			else ++x;
			if ((ch != '\t') && (ch != ' '))
				xts = x;
		}
	}
	return xts;
}

//#ifndef QT_MAC_USE_COCOA
void View::renderLine(QPainter* p, int yr, int x0, int x1, int xts, int y)
{
	Ref<Line> line = document_->cache()->get(y);
	int lineWidth = line->width();
	Ref<TextLayout, Owner> layout = line->layout_;
	
	bool validLayout = layout;
	if (validLayout) {
		validLayout = (line->wx_ == wx_);
		if (!validLayout)
			validLayout = (lineWidth < wx_) && (line->layout_->lineCount() <= 1);
	}
	
	if (line->dirty_ || (!validLayout) || printMode_) {
		QString text = document()->copySpan(y, 0/*x0*/, intMax/*x1*/, ' '/*gap*/, true/*replaceTabBySpace*/);
		if ((!layout) || printMode_)
			layout = new TextLayout(text, p->font(), p->device());
		else
			layout->setText(text);
		if (!printMode_) {
			layoutCache_->pushBack(layout);
			if (layoutCache_->length() > cacheCapacity_)
				layoutCache_->popFront();
		}
		
		/*
		layout->setCacheEnabled(true);
		layout->beginLayout();
		QTextLine layoutLine = layout->createLine();
		layoutLine.setLineWidth(intMax);
		layoutLine.setPosition(QPointF(0, PTE_VIEW_RASTER_DISPLACEMENT));
		layout->endLayout();
		line->layout_ = layout;
		line->dirty_ = false;
		line->numFragments_ = numFragments(line);*/
		
		{
			Ref<ChunkList> chunkList = line->chunkList_;
			QList<QTextLayout::FormatRange> formatList;
			for (int k = 0, n = chunkList->length(); k < n; ++k) {
				Ref<Chunk> chunk = chunkList->get(k);
				QTextCharFormat format;
				format.setForeground(chunk->style_->fgColor_);
				/*if (chunk->style_ != document_->defaultStyle())
					format.setBackground(chunk->style_->bgColor_);
						// HACK, there we have renderLineBackground()!*/
				format.setFontFixedPitch(true); // redundant HACK
				format.setFontKerning(false); // redundant HACK
				if (chunk->style_->bold_)
					format.setFontWeight(QFont::Bold);
				QTextLayout::FormatRange range;
				range.format = format;
				range.start = chunk->x0_;
				range.length = chunk->x1_ - chunk->x0_;
				formatList << range;
			}
			layout->setAdditionalFormats(formatList);
		}
		
		{
			QTextOption option = layout->textOption();
			option.setWrapMode(QTextOption::WrapAnywhere);
			// option.setFlags(QTextOption::IncludeTrailingSpaces);
			// option.setAlignment(Qt::AlignRight);
			layout->setTextOption(option);
			layout->setCacheEnabled(!printMode_);
			layout->beginLayout();
			int xf0 = 0, nc = lineWidth, j = 0;
			int leadingSpaces = 0;
			while (xf0 < nc) {
				int xf1 = xf0 + wx_;
				if (xf1 > nc) xf1 = nc;
				if (xf1 - xf0 - leadingSpaces <= 0) break; // empiric HACK
				QTextLine line = layout->createLine();
				line.setNumColumns(xf1 - xf0 - leadingSpaces);
				line.setPosition(QPointF(leadingSpaces * gw_, 0));
				if (line.textLength() > wx_) {
					leadingSpaces = line.textLength() - wx_;
				}
				else
					leadingSpaces = 0;
				if (line.textStart() + line.textLength() >= nc) break; // paranoid HACK
				++j;
				xf0 = xf1;
			}
			layout->endLayout();
			line->layout_ = layout;
			line->dirty_ = false;
			line->wx_ = wx_; // to support layout invalidation
			line->numFragments_ = numFragmentsFromWidth(lineWidth); // to support incremental updates
		}
		
	}
	
	QRect bbox = QRect(0, yr + metrics_->lineSpacing_, (x1 - x0) * gw_, gh_);
	if (p->hasClipping() && (!printMode_))
		if (!p->clipRegion().contains(bbox)) return;
	
	{
		// p->save();
		// p->setClipRect(bbox);
		// layout->draw(p, QPointF(-x0 * gw_, yr + metrics_->lineSpacing_));
		int nf = layout->lineCount();
		for (int j = 0; j < nf; ++j) {
			QTextLine line = layout->lineAt(j);
			if (((j + 1) * wx_ <= x0) || (x1 <= j * wx_))
				;
			else {
				// qDebug() << "j*wx_, line.textStart() =" << j*wx_ << "," << line.textStart();
				// (line.textLength() < wx_) && (line.textStart() + line.textLength() != lineWidth);
				line.draw(p, QPointF((j * wx_ - x0) * gw_, yr + metrics_->lineSpacing_ + PTE_VIEW_RASTER_DISPLACEMENT));
			}
		}
		// p->restore();
	}
	
	if (metrics_->showWhitespace_)
	{
		p->setPen(document_->defaultStyle()->fgColor_);
		
		Ref<ChunkList> chunkList = line->chunkList_;
		for (int k = 0, n = chunkList->length(); k < n; ++k) {
			Ref<Chunk> chunk = chunkList->get(k);
			if ((chunk->x1_ <= x0) || (x1 <= chunk->x0_)) // completely outside
				continue;
			
			int j0 = (chunk->x0_ < x0) ? x0 - chunk->x0_ : 0;
			int j1 = ((x1 < chunk->x1_) ? x1 : chunk->x1_) - (chunk->x0_ + j0);
			
			if (j1 > chunk->text_.length()) // to support tabs
				j1 = chunk->text_.length();
			
			QString text = chunk->text_.mid(j0, j1);
			int xr = (chunk->x0_ + j0 - x0) * gw_;
				
			int x = chunk->x0_;
			for (int j = 0; j < j1 - j0; ++j) {
				QChar ch = text.at(j);
				if ((ch == '\t') || ((x >= xts) && (ch == ' '))) {
					int yrb = yr + ga_ + PTE_VIEW_RASTER_DISPLACEMENT;
					p->drawLine(
						xr + j * gw_,     yrb + 1,
						xr + j * gw_ + 1, yrb + 1
					);
					p->drawLine(
						xr + j * gw_, yrb + 1,
						xr + j * gw_, yrb
					);
				}
				if (ch == '\t') x += line->tabWidth(chunk->x0_ + j);
				else ++x;
			}
		}
	}
}
//#else
#if 0
void View::renderLine(QPainter* p, int yr, int x0, int x1, int xts, int y)
{
	Ref<Line> line = document_->cache()->get(y);
	Ref<ChunkList> chunkList = line->chunkList_;
	
	yr -= PTE_VIEW_RASTER_DISPLACEMENT;
	
	for (int k = 0, n = chunkList->length(); k < n; ++k)
	{
		Ref<Chunk> chunk = chunkList->get(k);
		if ((chunk->x1_ <= x0) || (x1 <= chunk->x0_)) // completely outside
			continue;
		
		int j0 = (chunk->x0_ < x0) ? x0 - chunk->x0_ : 0;
		int j1 = ((x1 < chunk->x1_) ? x1 : chunk->x1_) - (chunk->x0_ + j0);
		
		if (j1 > chunk->text_.length()) // to support tabs
			j1 = chunk->text_.length();
		
		QString text = chunk->text_.mid(j0, j1);
		int xr = (chunk->x0_ + j0 - x0) * gw_;
		
		/*if ((chunk->style_->bgColor_ != Qt::transparent) && (chunk->style_->bgColor_ != document_->defaultStyle()->bgColor_)) {
			QBrush brush(chunk->style_->bgColor_);
			p->fillRect(xr, yr, (chunk->x1_ - chunk->x0_) * gw_, gh_, brush);
		}*/
		
		if (chunk->style_->underline_ != p->font().underline()) {
			QFont font = p->font();
			font.setUnderline(chunk->style_->underline_);
			p->setFont(font);
		}
		
		if (chunk->style_->bold_ != p->font().bold()) {
			QFont font = p->font();
			font.setBold(chunk->style_->bold_);
			p->setFont(font);
		}
		
		p->setPen(chunk->style_->fgColor_);
		p->drawText(xr, yr + ga_ - 1, text);
		
		if (metrics_->showWhitespace_) {
			p->setPen(document_->defaultStyle()->fgColor_);
			int x = chunk->x0_;
			for (int j = 0; j < j1 - j0; ++j) {
				QChar ch = text.at(j);
				if ( (ch == '\t') ||
				     ((x >= xts) && (ch == ' ')) ) {
					p->drawLine(xr + j * gw_, yr + ga_ - 1, xr + j * gw_ + 1, yr + ga_ - 1);
					p->drawLine(xr + j * gw_, yr + ga_ - 1, xr + j * gw_    , yr + ga_ - 2);
				}
				if (ch == '\t') x += document_->tabWidth(chunk->x0_ + j);
				else ++x;
			}
		}
	}
}
#endif

void View::renderLineBackground(QPainter* p, int yr, int x0, int x1, int xts, Ref<Line> line)
{
	Ref<ChunkList> chunkList = line->chunkList_;
	
	for (int k = 0, n = chunkList->length(); k < n; ++k)
	{
		Ref<Chunk> chunk = chunkList->get(k);
		if ((chunk->x1_ <= x0) || (x1 <= chunk->x0_)) // completely outside
			continue;
		
		int j0 = (chunk->x0_ < x0) ? x0 - chunk->x0_ : 0;
		int j1 = ((x1 < chunk->x1_) ? x1 : chunk->x1_) - (chunk->x0_ + j0);
		
		if (j1 > chunk->text_.length()) // to support tabs
			j1 = chunk->text_.length();
		
		QString text = chunk->text_.mid(j0, j1);
		int xr = (chunk->x0_ + j0 - x0) * gw_;
		
		if ((chunk->style_->bgColor_ != Qt::transparent) && (chunk->style_ != document_->defaultStyle())) {
			QBrush brush(chunk->style_->bgColor_);
			p->fillRect(xr, yr, (chunk->x1_ - chunk->x0_) * gw_ + 1, gh_, brush); // + 1, visual HACK
		}
	}
}

void View::renderLineLinkBackground(QPainter* p, int yr, int x0, int x1, Ref<Line> line)
{
	Ref<LinkList> linkList = line->linkList_;
	if (!linkList) return;
	
	p->setPen(QPen(QBrush(colors_->foregroundColor_, Qt::Dense4Pattern), 1));
	QBrush highlight(colors_->selectionColor_);
	
	for (int k = 0, n = linkList->length(); k < n; ++k)
	{
		Ref<Link> link = linkList->get(k);
		int n = link->x1_ - link->x0_;
		if (n > 0) {
			int j0 = (link->x0_ < x0) ? x0 - link->x0_ : 0;
			int xr = (link->x0_ + j0 - x0) * gw_;
			n -= j0;
			if (n > 0) {
				if (link == activeLink_)
					p->fillRect(xr, yr, n * gw_, gh_, highlight);
				p->drawLine(xr, yr + ga_ + 1, xr + n * gw_, yr + ga_ + 1);
			}
		}
	}
}

void View::renderLine(int y)
{
	Ref<Line> line = document_->cache()->get(y);
	
	if ((!line->pixmap_) || line->dirty_)
	{
		Pixmap* pm = 0;
		
		int nc = line->width();
		
		if (nc == 0) { // empty line
			line->dirty_ = false;
			line->pixmap_ = 0;
		}
		else { // render non-empty line
			pm = new Pixmap(nc * gw_, gh_);
			pm->fill(Qt::transparent);
			
			QPainter p(pm);
			
			p.setFont(metrics_->font_);
			// p.setRenderHint(QPainter::TextAntialiasing, metrics_->fontAntialiasing_);
			
			int xts = (metrics_->showWhitespace_) ? firstTrailingSpace(line) : 0;
			renderLine(&p, 0, 0, nc, xts, y);
			p.end();
			
			line->dirty_ = false;
			line->pixmap_ = pm;
			
			renderCache_->pushBack(pm);
			if (renderCache_->length() > cacheCapacity_)
				renderCache_->popFront();
		}
	}
}

void View::renderCursor(QPainter* p, int cy, int cx, int yr, int j)
{
	if ((j * wx_ <= cx) && (cx < (j + 1)  * wx_))
	{
		int xr = gw_ * (cx - j * wx_);
		crx_ = xr; cry_ = yr /*+ metrics_->lineSpacing_*/;
		crh_ = gh_ /*- metrics_->lineSpacing_*/;
		crw_ = ((metrics_->cursorStyle_ & BlockCursor) != 0) ? gw_ : metrics_->cursorWidth_;
	
		QString s;
		Ref<Style> style;
		if (p) {
			if ((metrics_->cursorStyle_ & InvertedCursor) != 0) {
				document()->getCell(cy, cx, &s, &style);
				if (s == QString()) s = " ";
			}
		}
		
		if (blink_) {
			if ((metrics_->cursorStyle_ & BlockCursor) != 0) {
				if (p) {
					p->fillRect(crx_, cry_, crw_, crh_, QBrush(style->fgColor_));
					if ((metrics_->cursorStyle_ & InvertedCursor) != 0)
						p->setPen(QPen(style->bgColor_));
					else
						p->setPen(colors_->cursorColor_);
					p->drawText(xr, yr + ga_ + PTE_VIEW_RASTER_DISPLACEMENT + metrics_->lineSpacing_, s);
				}
			}
			else if ((metrics_->cursorStyle_ & BeamCursor) != 0) {
				if (p) {
					QColor color = ((metrics_->cursorStyle_ & InvertedCursor) != 0) ? style->fgColor_ : colors_->cursorColor_;
					p->fillRect(crx_, cry_, crw_, crh_, QBrush(color));
				}
			}
		}
		else {
			if ((metrics_->cursorStyle_ & BlockCursor) != 0) {
				if (p) {
					QColor color = ((metrics_->cursorStyle_ & InvertedCursor) != 0) ? style->fgColor_ : colors_->cursorColor_;
					p->setPen(QPen(color));
					p->drawRect(crx_, cry_, crw_, crh_-1); // -1, visual HACK
				}
			}
		}
		
		crx_ += trx_;
		cry_ += try_;
	}
}

void View::renderBracketMatch(QPainter* p, int bmy, int bmx, int yr, int j)
{
	if ( ((j * wx_ <= bmx) && (bmx < (j + 1)  * wx_)) &&
	     ((metrics_->cursorStyle_ & BeamCursor) != 0) /* quality HACK */ )
	{
		int xr = gw_ * (bmx - j * wx_);
		QString s;
		Ref<Style> style;
		document()->getCell(bmy, bmx, &s, &style);
		if (s == QString()) s = " ";
		
		p->fillRect(xr, yr, gw_, gh_, QBrush(colors_->bracketMatchColor_));
		QFont font = p->font();
		font.setBold(true);
		p->setFont(font);
		p->setPen(style->fgColor_);
		p->drawText(xr, yr + ga_ + PTE_VIEW_RASTER_DISPLACEMENT + metrics_->lineSpacing_, s);
		font.setBold(false); // workaround HACK, because of side-effect with line numbers rendering
		p->setFont(font);
	}
}

void View::renderSelection(QPainter* p, int selya, int selxa, int selyb, int selxb, int yr, int y, int j, const QColor& color, bool currentLineHack)
{
	// render range
	int xr0 = 0;
	int xr1 = wx_ * gw_;
	
	// fragment range
	int x0 = wx_ * j;
	int x1 = wx_ * (j + 1);
	
	// clipped fragment range
	int x0c = x0, x1c = x1;
	
	if (y == selya) {
		if (x1 < selxa) { // fragment completely out
			xr0 = xr1;
			x0c = x1;
		}
		else if (x0 < selxa) { // fragment partly in
			xr0 = (selxa - x0) * gw_;
			x0c = selxa;
		}
	}
	
	if (y == selyb) {
		if (selxb <= x0) { // fragment completely out
			xr1 = xr0;
			x0c = x1;
		}
		else if (selxb < x1) { // fragment partly in
			xr1 = (selxb - x0) * gw_;
			x1c = selxb;
		}
	}
	
	if (currentLineHack) {
		if (y == cursorLine()) { // workaround HACK
			p->fillRect(-metrics_->blanking_, yr, xr0 + metrics_->blanking_, gh_, colors_->currentLineColor_);
			p->fillRect(xr1, yr, width() - trx_ - xr1, gh_, colors_->currentLineColor_);
		}
	} else {
		if (xr0 < xr1) {
			p->fillRect(xr0, yr, xr1 - xr0, gh_, color);
		}
	}
}

void View::renderTo(QPainter* p)
{
	// redundancy HACK, following four lines need to go into a function (see resizeEvent())
	int nl = widthOfLineNumbersColumn(); // width of line numbers column
	wy_ = (height() - 2 * metrics_->blanking_) / gh_;
	wx_ = (width() - 2 * metrics_->blanking_) / gw_ - nl;
	mx_ -= nl * gw_;
	if (wy_ < 0) wy_ = 0;
	if (wx_ < 0) wx_ = 0;
	if ((wx_ <= 0) || (wy_ <= 0)) return;
	
	if (p) {
		p->setFont(metrics_->font_);
		if (bgBuffer_.isNull() && (!printMode_))
			p->fillRect(nl * gw_, 0, width() - nl * gw_ + 1/*rounding compensation*/, height(), QBrush(defaultStyle()->bgColor_));
		
		// QRect tube(metrics_->blanking_ - 1, 0, wx_ * gw_, height());
		// p->setClipRect(tube);
	}
	
	int cy = cursorLine();
	int cx = cursorColumn();
	
	int bmy0 = -1, bmx0 = -1;
	int bmy1 = -1, bmx1 = -1;
	if (hasBracketMatch())
		getBracketMatch(&bmy0, &bmx0, &bmy1, &bmx1);
	
	int yo = windowOrigin();
	int ye = yo + wy_;
	if (ye > cache()->length()) ye = cache()->length();
	
	int selya, selxa;
	int selyb, selxb;
	getSelection(&selya, &selxa, &selyb, &selxb);
	
	bool bottomToTop = (preferTopToBottomRendering_ ? ((yo == cache()->length() - wy_) && (cache()->length() > wy_)) : (yo > 0)) && (!printMode_);
	
	int nfTotal = 0;
	if (tinyDocScrollRange_) {
		bottomToTop = (yo > 0);
		yo = 0;
		ye = wy_;
		if (ye > cache()->length()) ye = cache()->length();
	}
	
	// extend to render lines half in view
	if (!printMode_) {
		if (bottomToTop) { if (yo > 0) --yo; }
		else { if (ye < cache()->length()) ++ye; }
	}
	
	if (p) {
		if (nl > 0)
			p->fillRect(0, 0, nl * gw_, height(), QBrush(colors_->lineNumbersBgColor_));
	}
	
	trx_ = nl * gw_ + metrics_->blanking_;
	try_ = metrics_->blanking_;
	if (p)
		p->translate(trx_, try_);
	
	int yr = 0; // render offset
	if (bottomToTop) {
		if (preferTopToBottomRendering_)
			yr = height() - try_ - metrics_->blanking_ - gh_;
		else
			yr = (wy_-1) * gh_;
	}
	
	vya_ = intMax;
	vxa_ = intMax;
	vyb_ = 0;
	vxb_ = 0;
	
	for (
		int y = bottomToTop ? ye - 1 : yo;
		bottomToTop ? (y >= yo) : (y < ye);
		y += bottomToTop ? -1 : 1
	)
	{
		document_->updateTabWidths(y);
		
		Ref<Line> line = cache()->get(y);
		
		int nc = line->width();
		int nf = numFragmentsFromWidth(nc);
		nfTotal += nf + (nf == 0);
		
		bool renderCaching = (nc <= renderCachingMaxLineLength_) && renderCaching_ && (!printMode_);
		
		if ((p) && renderCaching) {
			// if (line->dirty_) qDebug() << "render: " << y;
			renderLine(y);
		}
		
		int yra = (bottomToTop ? -(nf - (nf != 0)) * gh_ : 0) + yr;
		bool nonInView = true;
		bool selectedLine = ((selya <= y) && (y <= selyb - (selxb == 0)));
		
		if (p)
			line->yr_ = yra + try_;
		
		if (printMode_) {
			// prevent lines from being broken at page border
			if (height() < yr + try_ + (nf + (nf == 0)) * gh_ + metrics_->blanking_)
				break;
		}
		
		if ((line->pixmap_) || (!p) || ((p) && (!renderCaching) && (nf > 0)))
		{
			QPixmap* pm = (line->pixmap_) ? line->pixmap_ : 0;
			int xts = renderCaching ? 0 : firstTrailingSpace(line);
			if ((nf == 0) && (!p)) nf = 1;
			
			if ((metrics_->wrapMode_ == NoWrap) && (nf > 1)) nf = 1;
			
			for (
				int j = bottomToTop ? nf - 1 : 0;
				bottomToTop ? j >= 0 : j < nf;
				j += bottomToTop ? -1 : 1
			)
			{
				bool outOfView = (yr + try_ + gh_ <= 0) || (height() <= yr + try_);
				nonInView = nonInView && outOfView;
				if (!outOfView) {
					bool partialOutOfView = (yr + try_ < 0) || (height() < yr + try_ + gh_);
					if (!partialOutOfView) {
						if (y < vya_) { vya_ = y; vxa_ = intMax; }
						if (y > vyb_) { vyb_ = y; vxb_ = 0; }
						if (y == vya_) { if (vxa_ > j * wx_) vxa_ = j * wx_; }
						if (y == vyb_) { if (vxb_ < (j + 1) * wx_) vxb_ = (j + 1) * wx_; }
					}
					
					if (!p) {
						if ((yr <= my_) && (my_ < yr + gh_))
						{
							if (mapExact_) {
								// find exact valid cursor position
								ty_ = -1;
								tx_ = -1;
								int mxj = mx_ + j * gw_ * wx_;
								Ref<ChunkList> chunkList = line->chunkList_;
								for (int i = 0; i < chunkList->length(); ++i) {
									Ref<Chunk> chunk = chunkList->get(i);
									if ((gw_ * chunk->x0_ <= mxj) && (mxj < gw_ * chunk->x1_)) {
										ty_ = y;
										tx_ = chunk->x0_;
										if (mxj < gw_ * (chunk->x0_ + chunk->text_.length()))
											tx_ = mxj / gw_;
										else
											tx_ = chunk->x1_;
										break;
									}
								}
							}
							else {
								// find nearest valid cursor position
								ty_ = y;
								tx_ = 0;
								int mxj = mx_ + j * gw_ * wx_;
								Ref<ChunkList> chunkList = line->chunkList_;
								for (int i = 0; i < chunkList->length(); ++i) {
									Ref<Chunk> chunk = chunkList->get(i);
									if (gw_ * chunk->x0_ <= mxj) {
										tx_ = chunk->x0_;
										if (mxj < gw_ * (chunk->x0_ + chunk->text_.length())) {
											tx_ = mxj / gw_;
											break;
										}
										else
											tx_ = chunk->x1_;
									}
								}
							}
							
						}
					}
					else {
						bool selectedLine = (selya <= y) && (y <= selyb - (selxb == 0));
						if ((cy == y) && (!selectedLine))
							p->fillRect(-metrics_->blanking_, yr, width() - trx_ + metrics_->blanking_, gh_, colors_->currentLineColor_);
						renderLineBackground(p, yr, j * wx_, (j + 1) * wx_, xts, line);
						renderLineLinkBackground(p, yr, j * wx_, (j + 1) * wx_, line);
						if (cy == y) {
							if (selectedLine)
								renderSelection(p, selya, selxa, selyb, selxb, yr, y, j, colors_->selectionColor_, true);
							if (metrics_->showLineNumbers_)
								p->fillRect(-trx_, yr, trx_ - metrics_->blanking_, gh_, colors_->lineNumbersBgColorBright_);
						}
						if (showFindResult_) {
							MatchList* matches = line->matches_;
							if (matches) {
								for (int i = 0; i < matches->length(); ++i) {
									Match m = matches->get(i);
									renderSelection(p, y, m.x0_, y, m.x1_, yr, y, j, colors_->matchColor_);
								}
							}
						}
						if (selectedLine)
							renderSelection(p, selya, selxa, selyb, selxb, yr, y, j, colors_->selectionColor_);
						if (renderCaching)
							p->drawPixmap(0, yr, *pm, j * wx_ * gw_, 0, wx_ * gw_, gh_);
						else
							renderLine(p, yr, j * wx_, (j + 1) * wx_, xts, y);
						if (!hasSelection_) {
							if (bmy0 == y)
								renderBracketMatch(p, bmy0, bmx0, yr, j);
							if (bmy1 == y)
								renderBracketMatch(p, bmy1, bmx1, yr, j);
						}
					}
					if (cy == y)
						renderCursor(p, cy, cx, yr, j);
				}
				yr += bottomToTop ? -gh_ : gh_;
			}
		}
		else {
			nonInView = false;
			bool partialOutOfView = (yr + try_ < 0) || (height() - gh_ < yr + try_);
			if (!partialOutOfView) {
				if (y < vya_) { vya_ = y; vxa_ = 0; }
				if (y > vyb_) { vyb_ = y; vxb_ = 0; }
			}
			if (cy == y) {
				if (!selectedLine)
					p->fillRect(-metrics_->blanking_, yr, width() - trx_ + metrics_->blanking_, gh_, colors_->currentLineColor_);
				if (metrics_->showLineNumbers_)
					p->fillRect(-trx_, yr, trx_ - metrics_->blanking_, gh_, colors_->lineNumbersBgColorBright_);
			}
			if (selectedLine) {
				// ugly double invocation HACK
				renderSelection(p, selya, selxa, selyb, selxb, yr, y, 0, colors_->selectionColor_);
				renderSelection(p, selya, selxa, selyb, selxb, yr, y, 0, colors_->selectionColor_, true);
			}
			if (cy == y)
				renderCursor(p, cy, cx, yr, 0);
			yr += bottomToTop ? -gh_ : gh_;
		}
		
		if ((p) && (!nonInView)) {
			int ln = document_->lineNumber(y);
			if (metrics_->showLineNumbers_ || (ln < 0))
			{
				if (ln >= 0) {
					ln += 1;
					QColor color = ((cy == y) /*|| selectedLine*/) ? colors_->lineNumbersFgColorBright_ : colors_->lineNumbersFgColor_;
					Ref<TextLayout, Owner> layout = annotationCache_->value(ln);
					if (layout) {
						if (layout->additionalFormats().at(0).format.foreground().color() != color)
							layout = 0;
					}
					if (!layout)  {
						QString text = QString("%1").arg(ln);
						if (!layout)
							layout = new TextLayout(text, metrics_->font_, p->device());
						QTextCharFormat format;
						format.setForeground(color);
						QTextLayout::FormatRange range;
						range.format = format;
						range.start = 0;
						range.length = text.length();
						layout->setAdditionalFormats(QList<QTextLayout::FormatRange>() << range);
						layout->beginLayout();
						QTextLine line = layout->createLine();
						line.setLineWidth(intMax);
						line.setPosition(QPointF(0, PTE_VIEW_RASTER_DISPLACEMENT));
						layout->endLayout();
						layout->setCacheEnabled(true);
						annotationCache_->define(ln, layout);
					}
					layout->draw(p, QPoint(-(numDigits(ln) + 1) * gw_ - metrics_->blanking_, yra + metrics_->lineSpacing_));
				}
				else {
					QRect rect(-trx_, yra, width(), gh_);
					QColor fg = colors_->lineNumbersFgColor_;
					QColor bg = colors_->lineNumbersBgColor_;
					QColor border = bg.darker(115);
					p->save();
					QFont font = p->font();
					font.setItalic(true);
					font.setBold(true);
					font.setPixelSize(font.pixelSize() - 1);
					p->setFont(font);
					if (p->pen().color() != fg)
						p->setPen(fg);
					if (p->brush().color() != bg)
						p->setBrush(bg);
					p->fillRect(rect, bg);
					p->fillRect(rect.left(), rect.top(), width(), 1, border);
					p->fillRect(rect.left(), rect.bottom(), width(), 1, border);
					rect.setLeft(rect.left() + trx_);
					rect.setRight(rect.right() - metrics_->blanking_);
					Ref<Document> doc = document_->matchStatus()->matchingLines_->get(y)->document_;
					if (doc->filePath() != QString()) {
						QFileInfo info = doc->fileInfo();
						p->drawText(rect, Qt::AlignCenter, info.fileName() + " - " + info.path());
					}
					else {
						p->drawText(rect, Qt::AlignCenter, doc->title());
					}
					p->restore();
				}
			}
		}
	}
	
	if (p) {
		if ((cache()->length() <= wy_) && (wy_ < nfTotal)) {
			if (!tinyDocScrollRange_) {
				tinyDocScrollRange_ = true;
				updateVerticalScrollRange();
				if (!preferTopToBottomRendering_)
					yScroll_->setValue(1);
			}
		}
		else {
			if (tinyDocScrollRange_) {
				tinyDocScrollRange_ = false;
				updateVerticalScrollRange();
			}
		}
	}
}

} // namespace pte
