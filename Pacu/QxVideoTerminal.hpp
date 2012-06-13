#ifndef PACU_QXVIDEOTERMINALDEVICE_HPP
#define PACU_QXVIDEOTERMINALDEVICE_HPP

#include <pte/Document.hpp>
#include <pte/View.hpp>
#include <pte/Palette.hpp>
#include "UseFtl.hpp"
#include "UsePte.hpp"
#include "QxStreamForwarder.hpp"
#include "QxProcessObserver.hpp"

#ifdef Q_WS_MAC
#ifndef QT_MAC_USE_COCOA
#if (0x040500 <= QT_VERSION) && (QT_VERSION < 0x040700)
#define PACU_IM_WORKAROUND 1
#endif
#endif
#endif
#ifndef PACU_IM_WORKAROUND
#define PACU_IM_WORKAROUND 0
#endif

namespace ftl {
	template<class T> class Stack;
}

namespace pacu
{

class EscapeSequenceSyntax;
class MessageSyntax;
class UriSyntax;

class QxVideoTerminal: public View
{
	Q_OBJECT
	
public:
	enum PaletteIndex {
		Black          = 0,
		Red            = 1,
		Green          = 2,
		Yellow         = 3,
		Blue           = 4,
		Magenta        = 5,
		Cyan           = 6,
		White          = 7,
		BrightBlack    = 8,
		BrightRed      = 9,
		BrightGreen    = 10,
		BrightYellow   = 11,
		BrightBlue     = 12,
		BrightMagenta  = 13,
		BrightCyan     = 14,
		BrightWhite    = 15,
		NumColors      = 16,
		DefaultFgColor = 7,
		DefaultBgColor = 0
	};
	
	QxVideoTerminal(
		QString shell = "",
		QString shellArgs = "",
		Ref<Palette> palette = 0,
		QWidget* parent = 0
	);
	~QxVideoTerminal();
	
	QWidget* widget(QWidget* parent = 0);
	
	static Ref<ViewColors, Owner> defaultColors();
	static Ref<ViewMetrics, Owner> defaultMetrics();
	
	QColor paletteColor(int colorIndex, bool fullscreen = false) const;
	void setPaletteColor(int colorIndex, QColor color, bool fullscreen = false);
	
	Ref<Process> shell() const;
	int shellExitCode() const;
	int numberOfLines() const;
	
	QString defaultShellName() const;
	QString activeProcess() const;
	QString windowTitle() const;
	QString user() const;
	QString host() const;
	QString fqHost() const;
	QString cwd() const;
	QString cwdSimplified() const;
	
signals:
	void bell();
	void shellTerminated();
	void hangup();
	void activeProcessChanged(const QString& name);
	void windowTitleChanged(const QString& title);
	void locationInfoChanged(const QString& user, const QString& host, const QString& cwd);
	void openFile(QString path);
	void fullscreenMode(bool on);
	
public slots:
	void setNumberOfLines(int numLines);
	void setAutoOpenNextLink(bool on);
	void setPalette(Ref<Palette> palette);
	
	void copy();
	void paste();
	void selectAll();
	void shellClear();
	void csusp();
	void cintr();
	void ceof();
	
private slots:
	void clearScreen();
	void idle();
	void receive(const QByteArray& data);
	void detectActiveProcess();
	void parseLine(int y);
	
private:
	void parseMessages(int y, QString line);
	void parseUris(int y, QString line);
	
protected:
	virtual void inputMethodEvent(QInputMethodEvent* event);
	
private:
	virtual int cursorLine() const;
	virtual int cursorColumn() const;
	
	static int styleIndex(int cif, int cig, bool fs, bool ul);
	void updateColorsContiguous(Ref<Palette> palette = 0);
	
	static int toInt(const Array<uint8_t, ShallowCopyArray>& data, int i0, int i1);
	void enterContiguousMode();
	void enterFullscreenMode();
	void setWindowSize(int wy, int wx);
	bool flushText();
	void flushText(const QString& span);
	void limitCursor();
	void process(const Array<uint8_t, ShallowCopyArray>& data, Ref<Token> token, bool* needsUpdate);
	void interpretLocationInfo(const QString& title);
	
	virtual bool isWord(QChar ch) const;
	
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	void forwardMouseEvent(QMouseEvent* event, int ty, int tx, bool moveEvent, bool mouseRelease);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);
	virtual void wheelEvent(QWheelEvent* event);
	virtual void focusInEvent(QFocusEvent* event);
	virtual void dragEnterEvent(QDragEnterEvent* event);
	virtual void dropEvent(QDropEvent* event);
	
	Ref<Process, Owner> shell_;
	QxProcessObserver* observer_;
	int ioUnit_;
	Ref<Stack<uint8_t>, Owner> controlBuffer_;
	Ref<Stack<uint8_t>, Owner> textBuffer_;
	Ref<Stack<uint8_t>, Owner> sequenceTypeBuffer_;
	enum { UndefType = 0, OscType = 1, CsiType = 2, TriType = 3 };
	char sequenceType_;
	QxStreamForwarder* forwarder_;
	QTimer* idleTimer_;
	Ref<EscapeSequenceSyntax, Owner> escapeSequenceSyntax_;
	Ref<MessageSyntax, Owner> messageSyntax_;
	Ref<UriSyntax, Owner> uriSyntax_;
	
	Ref<Document, Owner> documentContiguous_;
	Ref<Document, Owner> documentFullscreen_;
	
	Ref<Cache, Owner> cacheContiguous_;
	Ref<Cache, Owner> cacheFullscreen_;
	int cacheContiguousSize_;
	int cacheFullscreenSize_;
	inline int cacheSize() const { return (cache() == cacheContiguous_) ? cacheContiguousSize_ : cacheFullscreenSize_; }
	
	Ref<ViewColors, Owner> colorsContiguous_;
	Ref<ViewColors, Owner> colorsFullscreen_;
	
	typedef Array< Ref<Style, Owner> > Styles;
	Ref<Styles, Owner> styles_;
	
	QByteArray pendingData_;
	
	int cy_, cx_; // cursor position
	int scryo_; // scroll area orign
	int scrye_; // scroll area end
	
	int fgColor_; // current foreground color
	int bgColor_; // current background color
	bool underline_; // current underline state
	bool bright_;
	
	bool insertMode_;
	bool applicationMode_;
	bool altKeypadMode_;
	bool autoOpenNextLink_;
	bool mouseTracking_;
	bool mouseMotionTracking_;
	QPoint mousePosSaved_;
	
	// save / restore state variables
	bool saved_;
	int cy2_, cx2_;
	int fgColor2_;
	int bgColor2_;
	bool underline2_;
	bool bright2_;
	
	int wrapModeSaved_;
	bool activeProcessDetectionRunning_;
	QString defaultShellName_;
	QString activeProcess_;
	QString windowTitle_;
	QString user_, host_, cwd_;
	int yjo_; // first line generated by job
	
	bool shutdown_;
	
	Ref<Style, Owner> preeditStyle_;
	QString preeditString_;
	int preeditCursor_;
	#if PACU_IM_WORKAROUND
	bool ignoreNextKeyPressEvent_;
	#endif
};

} // namespace pacu

#endif // PACU_QXVIDEOTERMINALDEVICE_HPP
