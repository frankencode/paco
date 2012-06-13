#ifndef PTE_VIEW_HPP
#define PTE_VIEW_HPP

#include <QPointer>
#include <QWidget>
#include <ftl/Map.hpp>
#include <ftl/Queue.hpp>
#include "ViewMetrics.hpp"
#include "Document.hpp"
#include "Highlighter.hpp"

class QMenu;
class QScrollBar;
class QPrinter;

namespace pte
{

class ScrollMachine;

class View: public QWidget
{
	Q_OBJECT
	
public:
	enum WrapMode { NoWrap = 0, FixedColumnWrap = 1 };
	enum CursorStyle { NoCursor = 0, BlockCursor = 1, BeamCursor = 2, InvertedCursor = 4, BlinkingCursor = 8 };
	
	View(QWidget* parent = 0, Ref<ViewMetrics> metrics = 0, Ref<ViewColors> colors = 0);
	
	static Ref<ViewMetrics, Owner> defaultMetrics();
	static Ref<ViewColors, Owner> defaultColors();
	Ref<ViewColors> colors() const;
	Ref<ViewMetrics> metrics() const;
	void applyMetrics();
	void updateSize();
	
	Ref<Document> document() const;
	void setDocument(Ref<Document> document);
	
	Ref<Highlighter> highlighter() const;
	void setHighlighter(Ref<Highlighter> highlighter);
	
	QScrollBar* verticalScrollBar() const;
	void setVerticalScrollBar(QScrollBar* scrollBar);
	void updateVerticalScrollRange();
	
	bool showFindResult() const;
	
	bool renderCaching() const;
	void setRenderCaching(bool on);
	
	bool printMode() const;
	void setPrintMode(bool on);
	
	virtual int cursorLine() const;
	virtual int cursorColumn() const;
	virtual bool moveCursor(int cy, int cx);
	void keepInView(int cy = -1, int cx = -1);
	void blinkRestart();
	
	virtual bool hasBracketMatch() const;
	virtual void getBracketMatch(int* bmy0, int* bmx0, int* bmy1, int* bmx1);
	
	int windowLines() const;
	int windowColumns() const;
	int windowOrigin() const;
	void setWindowOrigin(int value);
	void setWindowSizeHint(int ny, int nx);
	
	virtual QSize sizeHint() const;
	
	void getViewRange(int* vya, int* vxa, int* vyb, int* vxb, bool* tinyDoc = 0);
	
	bool hasSelection() const;
	void getSelection(int* selya, int* selxa, int* selyb, int* selxb) const;
	void select(int sely0, int selx0, int sely1, int selx1);
	void selectAll();
	void unselect();
	QString copySelection() const;
	
	virtual bool isWord(QChar ch) const;
	QString wordUnderCursor() const;
	
	QString saveState();
	void restoreState(QString state);
	
	int glyphWidth() const;
	int glyphHeight() const;
	int glyphAscent() const;
	void mapMouseToTextPos(int mx, int my, int* ty, int* tx, bool mapExact = false);
	
	void updateAfterLineModified(int y);
	void updateAfterCursorMoved(int ya, int yb);
	
	void fitToPage(QSize page);
	bool drawPage(QPainter* p);
	void print(QPrinter* printer, Ref<ViewMetrics> metrics, bool pageHeader, bool pageBorder);
	
	QString css(Style* style, bool noBg = false);
	QString cssNoBg(Style* style);
	void exportHtml(QTextStream* textSink);
	
	QMenu* contextMenu() const;
	void setContextMenu(QMenu* contextMenu);
	
	bool commitLines() const;
	
	virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
	
signals:
	void hasSelectionChanged(bool value);
	void lineCommitted(int y);
	void gotoLinkRequest(Ref<Link, Owner> link);
	
public slots:
	void setColors(Ref<ViewColors> colors);
	void setMetrics(Ref<ViewMetrics> metrics);
	void setShowFindResult(bool on);
	void setCommitLines(bool on);
	void gotoLine(int y);
	
	void openLink();
	void firstLink();
	void lastLink();
	void previousLink();
	void nextLink();
	
private slots:
	void selectionAutoScroll();
	void scroll(int steps);
	
protected:
	typedef CustomList< Ref<Pixmap, Owner> > RenderCache;
	typedef CustomList< Ref<TextLayout, Owner> > LayoutCache;
	
	Ref<ViewMetrics> activeMetrics() const;
	virtual void metricsChanged();
	
	Ref<Cache> cache() const;
	// Ref<RenderCache> renderCache() const; // design HACK, we shouldn't expose it
	void cacheClear();
	int renderCachingMaxLineLength() const;
	
	bool leadingMargin() const;
	void setLeadingMargin(int numChars);
	
	Qt::CursorShape mouseCursorShape() const;
	void setMouseCursorShape(Qt::CursorShape shape);
	
	bool preferTopToBottomRendering() const;
	void setPreferTopToBottomRendering(bool on);
	
	bool tinyDocScrollRange() const;
	
	void startSelection(int sely0, int selx0);
	void extendSelection(int sely1, int selx1);
	void deleteSelection();
	
	bool expandToWord(int ty, int tx, int* x0, int* x1) const;
	
	void commitLine(int y);
	Ref<Link> activeLink() const;
	void setLinkRange(int ylo, int yle);
	void resetLinkRange();
	int linksInRange(int ylo, int yle) const;
	
	void showTip(const QString& text);
	void hideTip();
	
	QPoint cursorRenderPos();
	void renderTo(QPainter* p = 0);

	virtual void paintEvent(QPaintEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual void focusInEvent(QFocusEvent* event);
	virtual void focusOutEvent(QFocusEvent* event);
	virtual void hideEvent(QHideEvent* event);
	virtual bool event(QEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	#ifdef QT_MAC_USE_COCOA
	virtual void enterEvent(QEvent* event);
	#endif
	virtual void leaveEvent(QEvent* event);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);
	int stepsFromWheelEvent(QWheelEvent* event);
	virtual void wheelEvent(QWheelEvent* event);
	virtual void contextMenuEvent(QContextMenuEvent* event);
	
public slots:
	inline void highlighterYield() { document_->highlighterYield(); }
	inline void highlighterResume() { document_->highlighterResume(); }
	inline void highlighterRestart() { document_->highlighterRestart(); }
	inline bool highlighterSync(Time timeout = 0) { return document_->highlighterSync(timeout); }
	
private slots:
	void setYOrigin(int yo);
	void blink();
	
	void updateTotalNumberOfFragmentsBeforeModified(Ref<Delta> delta);
	void updateTotalNumberOfFragmentsAfterModified(Ref<Delta> delta);
	
private:
	void initializeTotalNumberOfFragments();
	void updateTotalNumberOfFragments();
	void followScrollPos(int yofia2);
	
	Ref<Link> linkBelowCursor(int mx, int my, int* y = 0);
	void watchLinkBelowCursor(int mx, int my);
	#ifdef QT_MAC_USE_COCOA
	void updateCursor();
	#endif
	
	Ref<Style> defaultStyle() const;
	void setHasSelection(bool value);
	
	int numDigits(int x, int b = 10) const;
	int lineNumbersNumDigits() const;
	int widthOfLineNumbersColumn() const;
	int numFragmentsFromWidth(int nc);
	int numFragments(int y);
	int numFragments(Ref<Line> line);
	int firstTrailingSpace(Ref<Line> line);
	void renderLine(QPainter* p, int yr, int x0, int x1, int xts, int y);
	// void renderLine(QPainter* p, int yr, int x0, int x1, int xts, Ref<Line> line);
	void renderLineBackground(QPainter* p, int yr, int x0, int x1, int xts, Ref<Line> line);
	void renderLineLinkBackground(QPainter* p, int yr, int x0, int x1, Ref<Line> line);
	void renderLine(int y);
	void renderCursor(QPainter* p, int cy, int cx, int yr, int j);
	void renderBracketMatch(QPainter* p, int bmy, int bmx, int yr, int j);
	void renderSelection(QPainter* p, int selya, int selxa, int selyb, int selxb, int yr, int y, int j, const QColor& color, bool currentLineHack = false);
	
	Ref<ViewMetrics, Owner> exposedMetrics_;
	Ref<ViewMetrics, Owner> metrics_;
	Ref<ViewColors, Owner> colors_;
	Ref<Document, Owner> document_;
	
	Ref<RenderCache, Owner> renderCache_;
	Ref<LayoutCache, Owner> layoutCache_;
	int cacheCapacity_;
	
	typedef Map< int, Ref<TextLayout, Owner> > AnnotationCache;
	Ref<AnnotationCache, Owner> annotationCache_;
	
	QScrollBar* yScroll_;
	QPointer<QMenu> contextMenu_;
	
	int leadingMargin_;
	Qt::CursorShape mouseCursorShape_;
	
	bool preferTopToBottomRendering_;
	bool showFindResult_;
	bool commitLines_;
	
	bool renderCaching_;
	int renderCachingMaxLineLength_;
	
	QPixmap bgBuffer_;
	
	bool printMode_;
	
	int wy_, wx_; // window dimensions
	qreal gw_, gh_; // glyph dimensions
	qreal ga_; // glyph ascent
	
	int yoli_; // vertical origin line index
	int yofir_; // vertical origin fragment index relative
	int yofia_; // vertical origin fragment index absolute
	int nft_; // number of fragments total
	typedef Map<int, int> WidthCounts;
	Ref<WidthCounts, Owner> widthCounts_;
	
	bool mapExact_;
	int mx_, my_; // mouse position for translation
	int ty_, tx_; // corresponding text position
	
	bool moveCursorOverloaded_;
	bool hasSelection_;
	int sely0_, selx0_;
	int sely1_, selx1_;
	
	QTimer* blinkTimer_;
	bool blink_;
	int trx_, try_;
	int crx_, cry_;
	int crw_, crh_;
	
	int vya_, vxa_;
	int vyb_, vxb_;
	bool tinyDocScrollRange_;
	
	QSize sizeHint_;
	
	bool leftMouseButtonPressed_;
	bool selectionAutoScroll_;
	
	Ref<Link, SetNull> activeLink_;
	int lineNumberOfActiveLink_;
	int ylo_, yle_; // link range
	
	bool wheelFirstTime_;
	Time wheelPreviousTime_;
	
	#ifdef QT_MAC_USE_COCOA
	QCursor ibeam_;
	#endif
	
	ScrollMachine* scrollMachine_;
};

} // namespace pte

#endif // PTE_VIEW_HPP
