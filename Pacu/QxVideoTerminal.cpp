#include <QThread>
#include <QApplication>
#include <QClipboard>
#include <QScrollBar>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMenu>
#include <QHBoxLayout>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/ttydefaults.h>
#include <ftl/streams>
#include <ftl/process>
#include <ftl/utils>
#include <ftl/Stack.hpp>
#include <pte/Palette.hpp>
#include "EscapeSequenceSyntax.hpp"
#include "MessageSyntax.hpp"
#include "UriSyntax.hpp"
#include "QxVideoTerminal.hpp"

#define PACU_DEBUG_TERMINAL 0

#if PACU_IM_WORKAROUND
#warning "Using workaround for broken input method code."
#endif

namespace pacu
{

QxVideoTerminal::QxVideoTerminal(QString shell, QString shellArgs, Ref<Palette> palette, QWidget* parent)
	: View(parent, defaultMetrics(), defaultColors()),
	  ioUnit_(0x10000),
	  controlBuffer_(new Stack<uint8_t>(ioUnit_)),
	  textBuffer_(new Stack<uint8_t>(ioUnit_ + 3/*max size of UTF8 fragment*/)),
	  sequenceTypeBuffer_(new Stack<uint8_t>(ioUnit_ /*save bet*/)),
	  sequenceType_(UndefType),
	  escapeSequenceSyntax_(new EscapeSequenceSyntax),
	  messageSyntax_(new MessageSyntax),
	  uriSyntax_(new UriSyntax),
	  documentContiguous_(new Document),
	  documentFullscreen_(new Document),
	  cacheContiguous_(documentContiguous_->cache()),
	  cacheFullscreen_(documentFullscreen_->cache()),
	  cacheContiguousSize_(intMax),
	  cacheFullscreenSize_(intMax),
	  colorsContiguous_(colors()),
	  colorsFullscreen_(defaultColors()),
	  styles_(new Styles(NumColors * NumColors * 4 /* = 1024*/)),
	  cy_(0), cx_(0),
	  scryo_(0),
	  fgColor_(DefaultFgColor),
	  bgColor_(DefaultBgColor),
	  underline_(false),
	  bright_(false),
	  insertMode_(false),
	  applicationMode_(false),
	  altKeypadMode_(false),
	  autoOpenNextLink_(false),
	  mouseTracking_(false),
	  mouseMotionTracking_(false),
	  saved_(false),
	  wrapModeSaved_(View::FixedColumnWrap),
	  yjo_(-1),
	  shutdown_(false),
	  preeditCursor_(0)
	  #if PACU_IM_WORKAROUND
	  , ignoreNextKeyPressEvent_(false)
	  #endif
{
	setAcceptDrops(true);
	setAttribute(Qt::WA_InputMethodEnabled, true);
	
	// setPreferTopToBottomRendering(false);
	setCommitLines(true);
	setMouseTracking(true); // for link tracking
	setDocument(documentContiguous_);
	
	documentContiguous_->setTitle("Terminal Log");
	documentFullscreen_->setTitle("Terminal Window");
	
	String path = shell.toUtf8().data();
	if (path == "") {
		path = Process::env("SHELL");
		if (path == "")
			path = "/bin/sh";
	}
	
	Ref<StringList, Owner> options;
	if (shellArgs != "")
		options = String(shellArgs.toUtf8().constData()).split(" ");
	
	defaultShellName_ = Path(path).fileName();
	activeProcess_ = defaultShellName_;
	activeProcessDetectionRunning_ = true;
	QTimer::singleShot(0, this, SLOT(detectActiveProcess()));
	
	{
		Ref<ProcessFactory, Owner> factory = new ProcessFactory;
		factory->setType(Process::SessionLeader);
		factory->setSignalMask(new SignalSet());
		factory->setExecPath(path);
		factory->setIoPolicy(Process::ForwardAll|Process::ForwardByPseudoTerminal);
		if (options)
			factory->setOptions(options);
		#ifdef Q_WS_MAC
		else
			factory->options()->append("-l");
		#endif
		factory->envMap()->define("TERM", "xterm-color"); // 'xterm-color' would be a hack, unknown under e.g. Solaris10
		factory->envMap()->define("PROMPT_COMMAND",
		#ifdef Q_WS_MAC
			"printf \"\\033]7;$USER@`hostname`:$PWD\\007\""
				// Leopard has no '-f' option, but it always defaults to canconical name
		#else
			"printf \"\\033]7;$USER@`hostname -f`:$PWD\\007\""
				// on Linux default is short name and therefore we have to pass '-f'
		#endif
		); // ensure window title sequence is generated
		// factory->envMap()->set("PS1", "\033[00;32m\\u@\\h\033[00m:\033[00;34m\\w\033[00m\\$ ");
		#ifdef Q_WS_MAC
		factory->envMap()->define("CLICOLORS", "1");
		#endif
			// PROMPT_COMMAND is bash environment variable,
			// using printf instead of echo to easily support csh
			// csh users will add 'alias precmd $PROMPT_COMMAND' to their ~/.tcshrc or ~/.cshrc
		// factory->envMap()->set("LC_ALL", "en_US.UTF-8");
		shell_ = factory->produce();
	}
	
	forwarder_ = new QxStreamForwarder(shell_->rawOutput(), ioUnit_, this);
	connect(forwarder_, SIGNAL(forward(const QByteArray&)), this, SLOT(receive(const QByteArray&)));
	forwarder_->start();
	
	idleTimer_ = new QTimer(this);
	connect(idleTimer_, SIGNAL(timeout()), this, SLOT(idle()));
	idleTimer_->setSingleShot(true);
	idleTimer_->start();
	
	observer_ = new QxProcessObserver(shell_, this);
	connect(observer_, SIGNAL(terminated()), this, SIGNAL(shellTerminated()));
	connect(observer_, SIGNAL(terminated()), this, SIGNAL(hangup()));
	observer_->start();
	
	// initialize palette to style mapping table
	for (int cif = 0; cif < NumColors; ++cif)
		for (int cig = 0; cig < NumColors; ++cig) {
			styles_->set(styleIndex(cif, cig, false, false), new Style(Qt::white, Qt::black, false, false));
			styles_->set(styleIndex(cif, cig, false, true), new Style(Qt::white, Qt::black, false, true));
			styles_->set(styleIndex(cif, cig, true, false), new Style(Qt::white, Qt::black, false, false));
			styles_->set(styleIndex(cif, cig, true, true), new Style(Qt::white, Qt::black, false, true));
		}
	
	// setting up default palette for fullscreen mode
	setPaletteColor(Black,         QColor::fromRgbF(0.00, 0.00, 0.00), true);
	setPaletteColor(Red,           QColor::fromRgbF(0.70, 0.09, 0.09), true);
	setPaletteColor(Green,         QColor::fromRgbF(0.09, 0.70, 0.09), true);
	setPaletteColor(Yellow,        QColor::fromRgbF(0.70, 0.41, 0.09), true);
	setPaletteColor(Blue,          QColor::fromRgbF(0.09, 0.09, 0.70), true);
	setPaletteColor(Magenta,       QColor::fromRgbF(0.70, 0.09, 0.70), true);
	setPaletteColor(Cyan,          QColor::fromRgbF(0.09, 0.70, 0.70), true);
	setPaletteColor(White,         QColor::fromRgbF(0.70, 0.70, 0.70), true);
	setPaletteColor(BrightBlack,   QColor::fromRgbF(0.41, 0.41, 0.41), true);
	setPaletteColor(BrightRed,     QColor::fromRgbF(1.00, 0.33, 0.33), true);
	setPaletteColor(BrightGreen,   QColor::fromRgbF(0.33, 1.00, 0.33), true);
	setPaletteColor(BrightYellow,  QColor::fromRgbF(1.00, 1.00, 0.33), true);
	setPaletteColor(BrightBlue,    QColor::fromRgbF(0.33, 0.33, 1.00), true);
	setPaletteColor(BrightMagenta, QColor::fromRgbF(1.00, 0.33, 1.00), true);
	setPaletteColor(BrightCyan,    QColor::fromRgbF(0.33, 1.00, 1.00), true);
	setPaletteColor(BrightWhite,   QColor::fromRgbF(1.00, 1.00, 1.00), true);
	
	// setting up default palette for contiguous mode
	if (palette) {
		setPalette(palette);
	}
	else {
		for (int i = 0; i < NumColors; ++i)
			setPaletteColor(i, paletteColor(i, true), false);
		
		updateColorsContiguous();
	}
	
	// modify view colors of fullscreen mode
	{
		Ref<Style> style = styles_->get(styleIndex(DefaultFgColor, DefaultBgColor, true, false));
		colorsFullscreen_->foregroundColor_ = style->fgColor_;
		colorsFullscreen_->backgroundColor_ = style->bgColor_;
		colorsFullscreen_->selectionColor_ = QColor::fromRgbF(0.3, 0.5, 1., 0.5);
	}
	
	connect(this, SIGNAL(lineCommitted(int)), this, SLOT(parseLine(int)));
}

QxVideoTerminal::~QxVideoTerminal()
{
	#ifndef NDEBUG
	// print("~QxVideoTerminal: Terminating shell...\n");
	#endif
	if (observer_->isRunning())
		try { shell_->kill(SIGHUP); } catch (...) {}
	#ifndef NDEBUG
	// print("~QxVideoTerminal: Waiting on forwarder_\n");
	#endif
	
	shutdown_ = true;
	forwarder_->readyRead()->release();
	forwarder_->wait(3000); // workaround HACK, wait max n s
	
	// QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
		// ensure correct transmission of hangup signal
	
	#ifndef NDEBUG
	// print("~QxVideoTerminal: Forwarder terminated\n");
	#endif
	observer_->wait(3000); // workaround HACK, wait max n s
	#ifndef NDEBUG
	// print("~QxVideoTerminal: Observer terminated, shellExitCode() = %%\n", shellExitCode());
	#endif
}

QWidget* QxVideoTerminal::widget(QWidget* parent)
{
	QWidget* carrier = new QWidget(parent);
	carrier->setAttribute(Qt::WA_DeleteOnClose);
	carrier->setFocusProxy(this);
	setVerticalScrollBar(new QScrollBar(Qt::Vertical));
	QHBoxLayout* row = new QHBoxLayout;
	row->setSpacing(0);
	row->setMargin(0);
	row->addWidget(this);
	row->addWidget(verticalScrollBar());
	carrier->setLayout(row);
	return carrier;
}

Ref<ViewColors, Owner> QxVideoTerminal::defaultColors()
{
	Ref<ViewColors, Owner> colors = new ViewColors;
	colors->currentLineColor_ = QColor::fromRgbF(1., 1., 1., 0.);
	return colors;
}

Ref<ViewMetrics, Owner> QxVideoTerminal::defaultMetrics()
{
	Ref<ViewMetrics, Owner> metrics = new ViewMetrics;
	metrics->showLineNumbers_ = false;
	metrics->showWhitespace_ = false;
	metrics->cursorStyle_ = BlockCursor|InvertedCursor;
	metrics->lineSpacing_ = 0;
	return metrics;
}

QColor QxVideoTerminal::paletteColor(int colorIndex, bool fullscreen) const
{
	return styles_->get(styleIndex(colorIndex, 0, fullscreen, false))->fgColor_;
}

void QxVideoTerminal::setPaletteColor(int colorIndex, QColor color, bool fullscreen)
{
	for (int cj = 0; cj < NumColors; ++cj) {
		styles_->get(styleIndex(colorIndex, cj, fullscreen, false))->fgColor_ = color;
		styles_->get(styleIndex(colorIndex, cj, fullscreen, true))->fgColor_ = color;
		styles_->get(styleIndex(cj, colorIndex, fullscreen, false))->bgColor_ = color;
		styles_->get(styleIndex(cj, colorIndex, fullscreen, true))->bgColor_ = color;
	}
}

Ref<Process> QxVideoTerminal::shell() const { return shell_; }
int QxVideoTerminal::shellExitCode() const { return observer_->exitCode(); }
int QxVideoTerminal::numberOfLines() const { return cacheContiguousSize_; }
void QxVideoTerminal::setNumberOfLines(int numLines) { cacheContiguousSize_ = numLines; }
void QxVideoTerminal::setAutoOpenNextLink(bool on) { autoOpenNextLink_ = on; }

void QxVideoTerminal::setPalette(Ref<Palette> palette)
{
	for (int i = 0; i < NumColors; ++i)
		setPaletteColor(i, palette->colorByIndex(i));
	updateColorsContiguous(palette);
	cacheClear();
	update();
}

QString QxVideoTerminal::defaultShellName() const { return defaultShellName_; }
QString QxVideoTerminal::activeProcess() const { return activeProcess_; }
QString QxVideoTerminal::windowTitle() const { return windowTitle_; }
QString QxVideoTerminal::user() const { return user_; }
QString QxVideoTerminal::host() const {
	QString host = fqHost().split(".").at(0);
	#ifdef Q_WS_MAC
	QStringList parts = host.toLower().split("-mac-");
	host = parts.at(parts.length() - 1);
	#endif
	return host;
}
QString QxVideoTerminal::fqHost() const { return host_; }
QString QxVideoTerminal::cwd() const { return cwd_; }
QString QxVideoTerminal::cwdSimplified() const {
	QString path = cwd();
	QString home = QString::fromUtf8(Process::env("HOME").utf8());
	if (path.toLower().indexOf(home.toLower()) == 0) /*case insensitive for OS X*/
		path = "~" + path.mid(home.length(), path.length() - home.length());
	return path;
}

int QxVideoTerminal::cursorLine() const { return cache()->length() - cy_ - 1; }
int QxVideoTerminal::cursorColumn() const { return cx_ + preeditCursor_; }

void QxVideoTerminal::copy()
{
	if (hasSelection())
		QApplication::clipboard()->setText(copySelection());
}

void QxVideoTerminal::paste()
{
	QString text = QApplication::clipboard()->text();
	if (!text.isEmpty()) {
		QByteArray ba = text.toUtf8();
		shell_->rawInput()->write(ba.data(), ba.size());
	}
}

void QxVideoTerminal::selectAll()
{
	select(0, 0, cache()->length()-1, intMax);
	update();
}

void QxVideoTerminal::csusp()
{
	uint8_t cc = CSUSP;
	shell_->rawInput()->write(&cc, 1);
}

void QxVideoTerminal::cintr()
{
	uint8_t cc = CINTR;
	shell_->rawInput()->write(&cc, 1);
}

void QxVideoTerminal::ceof()
{
	uint8_t cc = CEOF;
	shell_->rawInput()->write(&cc, 1);
}

int QxVideoTerminal::styleIndex(int cif, int cig, bool fs, bool ul)
{
	return cif + NumColors * (cig + NumColors * fs + 2 * NumColors * ul);
}

// update view colors of contiguous mode to match the default palette colors
void QxVideoTerminal::updateColorsContiguous(Ref<Palette> palette)
{
	Ref<Style> style = styles_->get(styleIndex(DefaultFgColor, DefaultBgColor, false, false));
	colorsContiguous_->foregroundColor_ = style->fgColor_;
	colorsContiguous_->backgroundColor_ = style->bgColor_;
	QColor selectionColor, matchColor;
	if (palette) {
		selectionColor = palette->colorByName("Selection");
		matchColor = palette->colorByName("Match");
	}
	if (selectionColor.isValid())
		colorsContiguous_->selectionColor_ = selectionColor;
	else
		colorsContiguous_->selectionColor_ = QColor::fromRgbF(0.3, 0.5, 1., 0.5);
	if (matchColor.isValid())
		colorsContiguous_->matchColor_ = matchColor;
	else
		colorsContiguous_->matchColor_ = QColor("#FF6");
	if (colors() == colorsContiguous_) // mighty HACK to achieve correct default document format
		setColors(colorsContiguous_);
}

int QxVideoTerminal::toInt(const Array<uint8_t, ShallowCopyArray>& data, int i0, int i1)
{
	int value = 0;
	for (int i = i0; i < i1; ++i) {
		value *= 10;
		value += data.get(i) - '0';
	}
	return value;
}

void QxVideoTerminal::enterContiguousMode()
{
	if (document() != documentContiguous_) {
		setColors(colorsContiguous_);
		setDocument(documentContiguous_);
		activeMetrics()->wrapMode_ = wrapModeSaved_;
		unselect();
		cacheClear();
		update();
		emit fullscreenMode(false);
	}
}

void QxVideoTerminal::enterFullscreenMode()
{
	if (document() != documentFullscreen_) {
		setColors(colorsFullscreen_);
		setDocument(documentFullscreen_);
		wrapModeSaved_ = activeMetrics()->wrapMode_;
		activeMetrics()->wrapMode_ = NoWrap;
		updateVerticalScrollRange();
		unselect();
		cacheClear();
		update();
		emit fullscreenMode(true);
	}
}

void QxVideoTerminal::setWindowSize(int wy, int wx)
{
	// inform session about size change
	// (SIGWINCH will be sent)
	if ((wy > 0) && (wx > 0)) {
		struct winsize ws;
		::memset(&ws, 0, sizeof(ws));
		ws.ws_row = wy;
		ws.ws_col = wx;
		::ioctl(shell_->rawOutput()->fd(), TIOCSWINSZ, &ws);
	}
}

bool QxVideoTerminal::flushText()
{
	if (textBuffer_->fill() == 0) return false;
	QString span = QString::fromUtf8((char*)textBuffer_->data(), textBuffer_->fill());
	flushText(span);
	textBuffer_->clear();
	return true;
}

void QxVideoTerminal::flushText(const QString& span)
{
	#if PACU_DEBUG_TERMINAL
	ftl::print("QxVideoTerminal::flushText(): span = \"%%\"\n", String(span.toUtf8()));
	#endif
	
	if (!insertMode_)
		document()->popSpan(-cy_ - 1, cx_, cx_ + span.length());
	Ref<Style> format = styles_->get(styleIndex(fgColor_, bgColor_, (document() == documentFullscreen_), underline_));
	// bool cxSaved = cx_;
	document()->pushSpan(-cy_ - 1, cx_, &cx_, span, format);
	#if 0
	if (document()->cache()->get(-cy_ - 1)->limitToScreen_) { // libreadline HACK
		int wx = windowColumns();
		if ((cxSaved <= wx) && (wx < cx_)) {
			QString fragment;
			document()->popSpan(-cy_ - 1, wx, intMax, &fragment);
			lineFeed();
			if (fragment.length() > 0) {
				if (!insertMode_)
					document()->popSpan(-cy_ - 1, 0, fragment.length());
				document()->pushSpan(-cy_ - 1, 0, &cx_, fragment);
			}
		}
	}
	#endif
}

void QxVideoTerminal::limitCursor()
{
	if (cy_ < 0) cy_ = 0;
	if (cy_ >= windowLines()) cy_ = windowLines() - 1;
	if (cx_ < 0) cx_ = 0;
}

void QxVideoTerminal::process(const Array<uint8_t, ShallowCopyArray>& data, Ref<Token> token, bool* needsUpdate)
{
	#if PACU_DEBUG_TERMINAL
	ftl::print("QxVideoTerminal::process(): data = ");
	for (int i = token->i0(), i1 = token->i1(); i < i1; ++i) {
		char ch = data.get(i);
		if (ch < 32)
			ftl::print("(%%)", uint8_t(ch));
		else
			ftl::print("%%", ch);
	}
	ftl::print("\n");
	#endif
	
	bool unsupported = false;
	
	if (data.get(token->i0() + 1) == '[')
	{
		char cmd = data.get(token->i1() - 1);
		
		/*if (cmd < 33)
			qDebug() << "cmd = (" << int(cmd) << ")";
		else
			qDebug() << "cmd =" << cmd;*/
		
		if (cmd == 'm') // set graphics rendition
		{
			Ref<Token> arg = token->firstChild();
			
			if (!arg) {  // reset to default
				fgColor_ = DefaultFgColor;
				bgColor_ = DefaultBgColor;
				underline_ = false;
				bright_ = false;
			}
			
			while (arg)
			{
				int value = toInt(data, arg->i0(), arg->i1());
				
				if (value == 0) { // reset to default
					fgColor_ = DefaultFgColor;
					bgColor_ = DefaultBgColor;
					underline_ = false;
					bright_ = false;
				}
				else if (value == 1) { // bright
					bright_ = true;
				}
				else if (value == 4) { // underline
					underline_ = true;
				}
				else if (value == 7) { // inverted
					fgColor_ = DefaultBgColor;
					bgColor_ = DefaultFgColor;
				}
				else if ((30 <= value) && (value <= 37)) { // foreground color
					fgColor_ = value - 30;
				}
				else if (value == 39) { // default foreground color
					fgColor_ = DefaultFgColor;
				}
				else if ((40 <= value) && (value <= 47)) { // background color
					bgColor_ = value - 40;
				}
				else if (value == 49) { // reset to default background color
					bgColor_ = DefaultBgColor;
				}
				else {
					#ifndef NDEBUG
					ftl::print("Unsupported graphics rendition code = %%\n", value);
					#endif
				}
				
				arg = arg->nextSibling();
			}
			
			if (bright_)
				fgColor_ |= 8;
		}
		else if (cmd == 'H') // move cursor
		{
			int numArgs = token->countChildren();
			
			if (numArgs == 2)
			{
				Ref<Token> arg = token->firstChild();
				cy_ = windowLines() - toInt(data, arg->i0(), arg->i1());
				arg = arg->nextSibling();
				cx_ = toInt(data, arg->i0(), arg->i1()) - 1;
				
				if (cx_ < 0) cx_ = 0;
				if (cx_ >= windowColumns()) cx_ = windowColumns() - 1;
				if (cy_ < 0) cy_ = 0;
				if (cy_ >= windowLines()) cy_ = windowLines() - 1;
				
				*needsUpdate = true;
			}
			else if (numArgs == 0)
			{
				cy_ = windowLines() - 1;
				cx_ = 0;
				
				*needsUpdate = true;
			}
			else
				unsupported = true;
		}
		else if (('A' <= cmd) && (cmd <= 'D')) // step cursor (A: up, B: down, C: right, D: left)
		{
			int numArgs = token->countChildren();
			
			if ((numArgs == 0) || (numArgs == 1))
			{
				Ref<Token> arg = token->firstChild();
				int value = (arg) ? toInt(data, arg->i0(), arg->i1()) : 1;
				if (cmd == 'A') { // step up
					if (document() == documentContiguous_) {
						int wx = windowColumns();
						while (value > 0) {
							bool canMove = false;
							while ((value > 0) && (cx_ >= wx)) {
								// step within fragments
								cx_ -= wx;
								--value;
								canMove = true;
							}
							if ((value > 0) && (cy_ < document()->numberOfLines() - 1)) {
								// step to next line
								++cy_;
								int wl = document()->lineLength(-cy_ - 1);
								cx_ = wx * (wl / wx) + (cx_ % wx);
								--value;
								canMove = true;
							}
							if (!canMove) break;
						}
					}
					else
						cy_ += value;
				}
				else if (cmd == 'B') { // step down
					if (document() == documentContiguous_) {
						int wx = windowColumns();
						while (value > 0) {
							bool canMove = false;
							int wl = document()->lineLength(-cy_ - 1);
							int nf = wl / wx + ((wl % wx) != 0); // number of fragments
							while ((value > 0) && (cx_ < (nf - 1) * wx)) {
								// step within fragments
								cx_ += wx;
								--value;
								canMove = true;
							}
							if ((value > 0) && (cy_ > 0)) {
								// step to next line
								--cy_;
								cx_ = cx_ % wx;
								--value;
								canMove = true;
							}
							if (!canMove) break;
						}
					}
					else
						cy_ -= value;
				}
				else if (cmd == 'C') // step right
					cx_ += value;
				else if (cmd == 'D') // step left
					cx_ -= value;
				
				limitCursor();
				*needsUpdate = true;
			}
			else
				unsupported = true;
			
		}
		else if (cmd == 'J') // clear screen
		{
			int numArgs = token->countChildren();
			if (numArgs == 0) // "clear" from cursor to end of screen
			{
				// "clear" is meant to mean:
				// overwrite all text with spaces from cursor to end of cache
				
				Ref<Line> textLine = cache()->back(cy_);
				Ref<ChunkList> chunkList = textLine->chunkList_;
				
				// find first affected chunk
				int k = 0;
				while (k < chunkList->length())
				{
					Ref<Chunk> chunk = chunkList->get(k);
					
					if (cx_ < chunk->x1_) {
						if (cx_ <= chunk->x0_)
							break;
						for (int j = cx_ - chunk->x0_; j < chunk->text_.length(); ++j)
							chunk->text_[j] = ' ';
						++k;
						break;
					}
					
					++k;
				}
				
				// clear all successive chunks
				int y = cy_;
				while (true)
				{
					while (k < chunkList->length()) {
						chunkList->get(k)->text_.fill(' ');
						textLine->dirty_ = true;
						++k;
					}
					if (y == 0) break;
					--y;
					textLine = cache()->back(y);
					chunkList = textLine->chunkList_;
					k = 0;
				}
				
				*needsUpdate = true;
			}
			else if (numArgs == 1)
			{
				Ref<Token> arg = token->firstChild();
				int value = toInt(data, arg->i0(), arg->i1());
				
				if (value == 2) // clear whole screen
				{
					// usability HACK, clear complete text buffer
					clearScreen();
				}
				else
					unsupported = true;
			}
			else
				unsupported = true;
		}
		else if (cmd == 'K') // erase line
		{
			int numArgs = token->countChildren();
			
			if (numArgs == 0) // erase line from cursor to end of line
			{
				document()->popSpan(-cy_ - 1, cx_, intMax);
				*needsUpdate = true;
			}
			else if (numArgs == 1)
			{
				Ref<Token> arg = token->firstChild();
				
				int value = toInt(data, arg->i0(), arg->i1());
				
				if (value == 1) // erase from start of line to cursor
				{
					document()->popSpan(-cy_ - 1, 0, cx_ /* HACK, +1 ? */);
					*needsUpdate = true;
				}
				else if (value == 2) // erase entire line
				{
					document()->popSpan(-cy_ - 1, 0, intMax);
					*needsUpdate = true;
				}
				else
					unsupported = true;
			}
			else
				unsupported = true;
		}
		else if (cmd == 'L') // insert line(s)
		{
			int numArgs = token->countChildren();
			
			if (numArgs <= 1)
			{
				Ref<Token> arg = token->firstChild();
				int n = (arg) ? toInt(data, arg->i0(), arg->i1()) : 1;
				for (int i = 0; i < n; ++i) {
					int j = cache()->length() - cy_ - 1;
					if ((cache()->length() == cacheSize()) || (scryo_ > 0)) {
						//ftl::print("L: scryo_ = %%\n", scryo_);
						cache()->remove(-scryo_-1);
					}
					cache()->insert(j, new Line(document()->tabWidth()));
				}
				cy_ += n;
				*needsUpdate = true;
			}
			else
				unsupported = true;
		}
		else if (cmd == 'M') // remove line(s)
		{
			int numArgs = token->countChildren();
			
			if (numArgs <= 1)
			{
				Ref<Token> arg = token->firstChild();
				int n = (arg) ? toInt(data, arg->i0(), arg->i1()) : 1;
				for (int j = 0, y = cache()->length()-cy_-1; j < n; ++j)
					cache()->remove(y);
				for (int i = 0; i < n; ++i)
					cache()->insert(-scryo_-1, new Line(document()->tabWidth()));
				*needsUpdate = true;
			}
			else
				unsupported = true;
		}
		else if (cmd == 'P') // remove characters in current line
		{
			int numArgs = token->countChildren();
			if ((numArgs == 0) || (numArgs == 1))
			{
				Ref<Token> arg = token->firstChild();
				int n = (arg) ? toInt(data, arg->i0(), arg->i1()) : 1;
				if (document() == documentContiguous_) {
					int wx = windowColumns();
					int x0 = wx * (cx_ / wx + 1);
					QString swap;
					document()->popSpan(-cy_ - 1, x0, intMax, &swap);
					document()->popSpan(-cy_ - 1, cx_, cx_ + n);
					if (swap.length() > 0) {
						int h;
						document()->pushSpan(-cy_ - 1, x0, &h, swap);
					}
				}
				else
					document()->popSpan(-cy_ - 1, cx_, cx_ + n);
				*needsUpdate = true;
			}
			else
				unsupported = true;
		}
		else if (cmd == 'r') // set/reset scroll region
		{
			int numArgs = token->countChildren();
			if (numArgs == 2)
			{
				Ref<Token> arg0 = token->firstChild();
				Ref<Token> arg1 = arg0->nextSibling();
				int yb = windowLines() - toInt(data, arg0->i0(), arg0->i1());
				int ya = windowLines() - toInt(data, arg1->i0(), arg1->i1());
				if ((ya < 0) || (yb < 0))
					unsupported = true;
				else {
					if ((ya == 0) && (yb == windowLines() - 1)) {
						scryo_ = 0;
						scrye_ = 0;
					}
					else {
						scryo_ = ya;
						scrye_ = yb;
					}
				}
			}
			else if (numArgs == 0)
			{
				scryo_ = 0;
				scrye_ = -1;
			}
			else
				unsupported = true;
		}
		else if (cmd == 'h') // set option flag to high state
		{
			int numArgs = token->countChildren();
			if (numArgs == 1)
			{
				Ref<Token> arg = token->firstChild();
				int mode = toInt(data, arg->i0(), arg->i1());
				
				if ((data.get(arg->i0()-1)) == '?')
				{
					if (mode == 47) {
						enterFullscreenMode();
					}
					else if (mode == 1) {
						applicationMode_ = true;
					}
					else if (mode == 111) {
						autoOpenNextLink_ = true;
					}
					else if (mode == 1000) {
						mouseTracking_ = true;
						setMouseCursorShape(Qt::ArrowCursor);
					}
					else if ((mode == 1002) || (mode == 1003)) {
						mouseTracking_ = true;
						mouseMotionTracking_ = true;
						mousePosSaved_ = QPoint();
						setMouseCursorShape(Qt::ArrowCursor);
					}
					else
						unsupported = true;
				}
				else if ((data.get(arg->i0()-1)) == '[')
				{
					if (mode == 4)
						insertMode_ = true;
					else
						unsupported = true;
				}
				else
					unsupported = true;
			}
			else
				unsupported = true;
		}
		else if (cmd == 'l') // set option flag to low state
		{
			int numArgs = token->countChildren();
			if (numArgs == 1)
			{
				Ref<Token> arg = token->firstChild();
				int mode = toInt(data, arg->i0(), arg->i1());
				
				if ((data.get(arg->i0()-1)) == '?')
				{
					if (mode == 47) {
						enterContiguousMode();
					}
					else if (mode == 1) {
						applicationMode_ = false;
					}
					else if (mode == 111) {
						autoOpenNextLink_ = false;
					}
					else if ((mode == 1000) || (mode == 1002) || (mode == 1003)) {
						mouseTracking_ = false;
						mouseMotionTracking_ = false;
						setMouseCursorShape(Qt::IBeamCursor);
					}
					else
						unsupported = true;
				}
				else if ((data.get(arg->i0()-1)) == '[')
				{
					if (mode == 4)
						insertMode_ = false;
					else
						unsupported = true;
				}
				else
					unsupported = true;
			}
			else
				unsupported = true;
		}
		#if 0
		else if (cmd == 'c') // ask for VT100/VT220 version
		{
			int numArgs = token->countChildren();
			
			if (numArgs == 0)
			{}
			else if (numArgs == 1)
			{
				Ref<Token> arg = token->firstChild();
				int value = toInt(data, arg->i0(), arg->i1());
				if (value != 0)
					unsupported = true;
			}
			else
				unsupported = true;
				
			if (!unsupported) {
				// reply: I'm a VT220 of revision 7.0
				QByteArray ba("\033[>1;7;0c");
				shell_->rawOutput()->write(data.data(), ba.size());
			}
		}
		#endif
		else {
			unsupported = true;
		}
	}
	else if (data.get(token->i0() + 1) == ']')
	{
		char cmd = data.get(token->i1() - 1);
		if (cmd == 7) // xterm-style "operating system command"
		{
			int numArgs = token->countChildren();
			
			if (numArgs == 2)
			{
				Ref<Token> arg = token->firstChild();
				int value = toInt(data, arg->i0(), arg->i1());
				if (value == 0) { // set window title
					arg = arg->nextSibling();
					windowTitle_ = QString::fromUtf8((const char*)data.data() + arg->index(), arg->length());
					interpretLocationInfo(windowTitle_);
					emit windowTitleChanged(windowTitle_);
				}
				else if (value == 7) { // prompt command
					{
						if (yjo_ != -1) {
							int yje = cache()->length() - cy_ - 1;
							if (linksInRange(yjo_, yje) > 0) {
								setLinkRange(yjo_, yje);
								if (autoOpenNextLink_) {
									firstLink();
									openLink();
								}
							}
						}
						yjo_ = cache()->length() - cy_;
					}
					autoOpenNextLink_ = false;
					arg = arg->nextSibling();
					interpretLocationInfo(QString::fromUtf8((const char*)data.data() + arg->index(), arg->length()));
				}
				else if (value == 9) { // open file
					arg = arg->nextSibling();
					QString path = QString::fromUtf8((const char*)data.data() + arg->index(), arg->length());
					if ((path.length() > 1) && (cwd_.length() > 0)) {
						if (path.left(2) == "./")
							path = cwd_ + path.right(path.length() - 1);
					}
					// qDebug() << "(QxVideoTerminal) path = " << path;
					emit openFile(path);
				}
				else
					unsupported = true;
			}
			else
				unsupported = true;
		}
		else
			unsupported = true;
	}
	else if (data.get(token->i0() + 1) == ')')
	{
		if (data.get(token->i0() + 2) == '0')
		{}
		else
			unsupported = true;
	}
	else
	{
		char cmd = data.get(token->i0() + 1);
		
		if (cmd == 'M') // reverse line feed
		{
			++cy_;
			int wy = windowLines();
			if ((cy_ == wy) && (wy > 0)) {
				if (cache()->length() > wy)
					cache()->get(-wy-1)->chunkList_->clear();
				cache()->popBack();
				cache()->insert(-wy, new Line(document()->tabWidth()));
				--cy_;
			}
			else if (cy_ == scrye_) {
				// HACK, scroll region handling missing here
				unsupported = true;
			}
			
			*needsUpdate = true;
		}
		else if (cmd == '7') // save state
		{
			saved_ = true;
			cy2_ = windowLines() - cy_ - 1;
			cx2_ = cx_;
			fgColor2_ = fgColor_;
			bgColor2_ = bgColor_;
			underline2_ = underline_;
			bright2_ = bright_;
		}
		else if (cmd == '8') // restore state
		{
			if (saved_) {
				cy_ = windowLines() - cy2_ - 1;
				if (cy_ < 0) cy_ = 0;
				if ((cy_ >= windowLines()) || (cy_ >= cache()->length())) cy_ = windowLines() - 1;
				cx_ = cx2_;
				fgColor_ = fgColor2_;
				bgColor_ = bgColor2_;
				underline_ = underline2_;
				bright_ = bright2_;
				scryo_ = 0;
				scrye_ = 0;
				*needsUpdate = true;
			}
		}
		else if ((cmd == '=') || (cmd == '>'))
		{
			altKeypadMode_ = (cmd == '=');
		}
		else
			unsupported = true;
	}
	
	#ifndef NDEBUG
	if (unsupported)
	{
		ftl::print("Unsupported control sequence (cy_, cx_ = %%, %%): ", cy_, cx_);
		int i;
		for (i = token->index(); (i < token->index() + 10) && (i < data.length()); ++i) {
			char ch = data.get(i);
			if (ch > 32)
				ftl::print("%%", char(data.get(i)));
			else
				ftl::print("(%%)", int(data.get(i)));
		}
		ftl::print("\n");
	}
	#endif
}

void QxVideoTerminal::interpretLocationInfo(const QString& title)
{
	if (title.indexOf('@') < title.indexOf(':')) {
		QString location = title;
		if (location.contains('['))
			location.remove('[').remove(']');
		QStringList parts = location.split('@');
		if (parts.length() == 2) {
			user_ = parts.at(0);
			if (user_.contains(' ')) {
				QStringList pair = user_.split(' ');
				// activeProcess_ = pair.at(0);
				user_ = pair.at(1);
			}
			parts = parts.at(1).split(':');
			host_ = parts.at(0);
			cwd_ = parts.at(1).trimmed();
			// qDebug() << user_ << host_ << cwd_;
			emit locationInfoChanged(user_, host_, cwd_);
		}
	}
}

void QxVideoTerminal::idle()
{
	for (int i = 0; i < 64; ++i)
		forwarder_->readyRead()->release();
}

void QxVideoTerminal::receive(const QByteArray& newData)
{
	if (shutdown_) return;
	// ftl::print("~~~~~~~~~~\n");
	
	if (!forwarder_->readyRead()->tryAcquire())
		idleTimer_->start();
	else
		forwarder_->readyRead()->release();
	
	QByteArray data(newData);
	
	if (!pendingData_.isEmpty()) {
		data = pendingData_ + data;
		pendingData_.clear();
	}
	
	// check for UTF8 sequence of being broken up at buffer boundary
	{
		int i = data.length() - 1;
		if (uint8_t(data.at(i)) > 128)
		{
			// 0x8 = 1000
			// 0xC = 1100
			// 0xE = 1110
			// 0xF = 1111
		
			// scan back to the begin of sequence
			while (i >= 0) {
				if ((uint8_t(data.at(i)) & 0xC0) != 0x80)
					break;
				--i;
			}
			
			int n = data.size() - i;
			
			if (i == -1) { // sequence cut in the middle
				pendingData_ = data;
				data.clear();
			}
			else if (((uint8_t(data.at(i)) & 0xE0) == 0xC0) && (n < 2)) { // unfinished 2-byte sequence
				// qDebug() << "unfinished 2-byte sequence";
				pendingData_ = QByteArray(data.constData() + i, n);
				data.chop(n);
			}
			else if (((uint8_t(data.at(i)) & 0xF0) == 0xE0) && (n < 3)) { // unfinished 3-byte sequence
				// qDebug() << "unfinished 3-byte sequence";
				pendingData_ = QByteArray(data.constData() + i, n);
				data.chop(n);
			}
			else if (((uint8_t(data.at(i)) & 0xF8) == 0xF0) && (n < 4)) { // unfinshed 4-byte sequence
				// qDebug() << "unfinished 4-byte sequence";
				pendingData_ = QByteArray(data.constData() + i, n);
				data.chop(n);
			}
		}
	}
	
	if (data.isEmpty()) return;
	
	bool needsUpdate = false;
	bool atEnd = verticalScrollBar()->value() >= verticalScrollBar()->maximum() - tinyDocScrollRange();
	
	for (int i = 0, n = data.length(); i < n;)
	{
		uint8_t ch = uint8_t(data.at(i++));
		
		// ftl::print("QxVideoTerminal::receive(): ch = '%%'(%%)\n", char(ch), ch);
		
		if ((ch < 32) && (!((ch == 7) && (sequenceType_ == OscType))))
		{
			#if PACU_DEBUG_TERMINAL
			if (ch != 27)
				ftl::print("QxVideoTerminal::receive(): ch = (%%)\n", ch);
			#endif
			
			// filter control codes
			// (may appear at any point inside normal text stream or control sequence)
			
			if (ch == 13) { // carriage return
				flushText();
				// ftl::print("before cr: %%, %%, %%\n", cy_, cx_, windowColumns());
				if (document() == documentContiguous_) {
					int wx = windowColumns();
					int wl = document()->lineLength(-cy_ - 1);
					if (wl <= wx)
						cx_ = 0;
					else
						cx_ -= cx_ % wx;
					limitCursor();
				}
				else
					cx_ = 0;
				// ftl::print("after cr: %%, %%, %%\n", cy_, cx_, windowColumns());
				#if 0
				if ((document()->lineLength(-cy_ - 1) == wx + 1) && (cx_ > wx)) { // libreadline HACK
					// Implicit linefeed, which should have been triggered already by
					// writing beyond the right border. This is typewriter logic to
					// support old-school libreadline based applications.
					QString fragment;
					fragment = document()->copySpan(-cy_-1, wx, intMax);
					if (fragment.at(0) == ' ') {
						document()->popSpan(-cy_ - 1, wx, intMax, &fragment);
						document()->cache()->get(-cy_ - 1)->limitToScreen_ = true;
						lineFeed();
					}
				}
				cx_ = 0;
				#endif
				needsUpdate = true;
			}
			else if (ch == 10) { // line feed
				flushText();
				//ftl::print("before lf: %%, %%, %%\n", cy_, cx_, windowColumns());
				bool done = false;
				if (document() == documentContiguous_) {
					int wx = windowColumns();
					int wl = document()->lineLength(-cy_ - 1);
					int nf = wl / wx + ((wl % wx) != 0); // number of fragments
					if (cx_ < (nf - 1) * wx) {
						cx_ += wx;
						done = true;
					}
				}
				if (!done) {
					if (document() != documentFullscreen_)
						commitLine(cache()->length() - cy_ - 1);
					--cy_;
					if ((scryo_ != 0) && (cy_ == scryo_-1)) {
						cache()->remove(-scrye_-1);
						cache()->insert(-scryo_-1, new Line(document()->tabWidth()));
						// qDebug() << "LF: insert(-scryo_-1, new Line)";
						++cy_;
					}
					else if (cy_ < 0) {
						if (cache()->length() == cacheSize())
							cache()->popFront();
						// qDebug() << "LF: pushBack(new Line)";
						cache()->pushBack(new Line(document()->tabWidth()));
						++cy_;
					}
					// document()->cache()->get(-cy_ - 1)->limitToScreen_ = false; // libreadline HACK
				}
				//ftl::print("after lf: %%, %%, %%\n", cy_, cx_, windowColumns());
				limitCursor();
				if (document() == documentContiguous_) {
					// another nasty and difficult to appreciate HACK
					// (required to get carriage returns right although contradicting dynamic line breaking)
					int wl = document()->lineLength(-cy_ - 1);
					if (cx_ > wl) cx_ = wl;
				}
				needsUpdate = true;
			}
			else if (ch == 8) { // "backspace", step left one column
				flushText();
				if (cx_ > 0) --cx_;
				needsUpdate = true;
			}
			else if (ch == 9) { // tabulator step
				flushText();
				cx_ += 8 - cx_ % 8;
				needsUpdate = true;
			}
			else if (ch == 7) {
				emit bell();
			}
			else if (ch == 27) {
				if (flushText())
					needsUpdate = true;
				controlBuffer_->push(27);
			}
			else {
				// HACK, silently discard unhandled control code
			}
			
			continue;
		}
		
		if (controlBuffer_->fill() > 0)
		{
			if (controlBuffer_->fill() == controlBuffer_->size()) {
				controlBuffer_->clear();
				sequenceTypeBuffer_->clear();
				sequenceType_ = UndefType;
				// HACK, silently discard an over-long control sequence
				// (and garble the rest as text through!)
			}
			else {
				if (controlBuffer_->top() == 27) {
					if (ch == '[')  // VTxxx style CSI
						sequenceType_ = CsiType;
					else if (ch == ']') // xterm-style OSC
						sequenceType_ = OscType;
					else if ((ch == ')') || (ch == '(') || (ch == '%')) // obsolete glyph selectors, etc.
						sequenceType_ = TriType;
					else // else this is a short two byte control 'sequence'
						sequenceType_ = UndefType;
					sequenceTypeBuffer_->push(sequenceType_);
				}
				
				controlBuffer_->push(ch);
				
				bool eos = (sequenceType_ == UndefType); // end of sequence
				
				if (sequenceType_ == OscType)
					eos = (ch == 7);
				else if (sequenceType_ == CsiType)
					eos = (('A' <= ch) && (ch <= 'Z')) || (('a' <= ch) && (ch <= 'z'));
				else if (sequenceType_ == TriType)
					eos = (controlBuffer_->fill() == 3);
				
				if (eos) {
					flushText();
					uint8_t* sequence = controlBuffer_->data() + controlBuffer_->fill() - 1;
					int sequenceLength = 1;
					while (*sequence != 27) {
						--sequence;
						++sequenceLength;
					}
					sequenceTypeBuffer_->pop();
					Array<uint8_t, ShallowCopyArray> data(sequence, sequenceLength);
					Ref<Token, Owner> rootToken = escapeSequenceSyntax_->match(&data);
					if (rootToken)
						process(data, rootToken, &needsUpdate);
					
					if (sequenceTypeBuffer_->fill() > 0)
						sequenceType_ = sequenceTypeBuffer_->top();
					else
						sequenceType_ = UndefType;
					// qDebug() << "sequenceLength, controlBuffer_->fill() = " << sequenceLength << controlBuffer_->fill();
					controlBuffer_->popMore(sequenceLength);
					needsUpdate = true;
				}
			}
		}
		else {
			textBuffer_->push(ch);
		}
	}
	
	if (flushText())
		needsUpdate = true;
	
	updateVerticalScrollRange();
	
	if (atEnd) {
		if ((!tinyDocScrollRange()) || (cy_ == 0)) // uncertain usability HACK
			if (verticalScrollBar()->value() != verticalScrollBar()->maximum())
				verticalScrollBar()->setValue(verticalScrollBar()->maximum());
	}
	
	blinkRestart();
	
	if (needsUpdate)
		update();
	
	if (!activeProcessDetectionRunning_) {
		activeProcessDetectionRunning_ = true;
		QTimer::singleShot(0, this, SLOT(detectActiveProcess()));
	}
}

void QxVideoTerminal::detectActiveProcess()
{
	try {
		ProcessStatus shellStatus(shell_->id());
		if (shellStatus.commandName() != "") {
			if (shellStatus.processGroupId() == shellStatus.foregroundProcessGroupId()) {
				String name = shellStatus.commandName();
				if (activeProcess_ != name) {
					activeProcess_ = name;
					emit activeProcessChanged(activeProcess_);
				}
			}
			else {
				ProcessStatus jobStatus(shellStatus.foregroundProcessGroupId());
				if (jobStatus.commandName() != "") {
					String name = jobStatus.commandName();
					if (activeProcess_ != name) {
						activeProcess_ = name;
						emit activeProcessChanged(activeProcess_);
					}
				}
			}
		}
		activeProcessDetectionRunning_ = false;
	}
	catch (AnyException& ex) {
		QTimer::singleShot(0, this, SLOT(detectActiveProcess()));
		activeProcessDetectionRunning_ = true;
	}
}

void QxVideoTerminal::parseLine(int y)
{
	QString line = document()->copySpan(y, 0, intMax);
	// qDebug() << QString("QxVideoTerminal::parseLine(): line = \"%1\"").arg(line);
	parseMessages(y, line);
	parseUris(y, line);
}

void QxVideoTerminal::parseMessages(int y, QString line)
{
	StringAdapter media(&line);
	int i0 = 0, i1 = -1;
	Ref<Token, Owner> rootToken;
	while (rootToken = messageSyntax_->find(&media, &i0, &i1)) {
		QString targetPath;
		int targetRow = -1;
		int targetColumn = -1;
		Ref<Token> token = rootToken->firstChild();
		while (token) {
			if (token->rule() == messageSyntax_->path_) {
				targetPath = line.mid(token->index(), token->length());
			}
			else if (token->rule() == messageSyntax_->row_) {
				targetRow = line.mid(token->index(), token->length()).toInt();
			}
			else if (token->rule() == messageSyntax_->col_) {
				targetColumn = line.mid(token->index(), token->length()).toInt();
			}
			token = token->nextSibling();
		}
		Ref<Link, Owner> link = new Link(Link::FilePos, i0, i1, targetPath, targetRow, targetColumn);
		Ref<Line> textLine = document()->cache()->get(y);
		if (!textLine->linkList_) textLine->linkList_ = new LinkList;
		textLine->linkList_->append(link);
		// qDebug() << QString("QxVideoTerminal::parseLine(): [%1], n = %2").arg(line.mid(i0, i1-i0)).arg(i1-i0);
		i0 = i1;
	}
}

void QxVideoTerminal::parseUris(int y, QString line)
{
	StringAdapter media(&line);
	int i0 = 0, i1 = -1;
	Ref<Token, Owner> rootToken;
	while (rootToken = uriSyntax_->find(&media, &i0, &i1)) {
		Ref<Token> schemeToken = rootToken->firstChild();
		QString scheme = line.mid(schemeToken->index(), schemeToken->length());
		if ( (scheme == "http")  ||
		     (scheme == "https") ||
		     (scheme == "ftp")   ||
		     (scheme == "ftps")  ||
		     (scheme == "fish")  ||
		     (scheme == "file") ) {
			Ref<Link, Owner> link = new Link(Link::Uri, i0, i1, line.mid(rootToken->index(), rootToken->length()));
			Ref<Line> textLine = document()->cache()->get(y);
			if (!textLine->linkList_) textLine->linkList_ = new LinkList;
			textLine->linkList_->append(link);
		}
		i0 = i1;
	}
}

bool QxVideoTerminal::isWord(QChar ch) const
{
	return (('a' <= ch) && (ch <= 'z')) ||
	       (('A' <= ch) && (ch <= 'Z')) ||
	       (('0' <= ch) && (ch <= '9')) ||
	       (ch == '_') ||
	       (ch == '-') ||
	       (ch == '.') ||
	       (ch == '~') ||
	       (ch == QDir::separator());
}

void QxVideoTerminal::resizeEvent(QResizeEvent* event)
{
	// workaround HACK, filter illegal resize events
	if ((event->size().height() == 0) || (event->size().width() == 0))
		return;
	
	bool atEnd = verticalScrollBar()->value() == verticalScrollBar()->maximum();
	int wy2 = windowLines();
	int wx2 = windowColumns();
	
	View::resizeEvent(event);
	
	int wy = windowLines();
	int wx = windowColumns();
	int wyDelta = wy - wy2;
	
	// resize fullscreen cache
	{
		while (cacheFullscreen_->length() > wy)
			cacheFullscreen_->popFront();
		cacheFullscreenSize_ = wy;
		while (cacheFullscreen_->length() < wy)
			cacheFullscreen_->pushBack(new Line(document()->tabWidth()));
	}
	
	// resize contiguous cache
	{
		check(cacheContiguousSize_ >= wy);
		
		// remove empty lines, when reducing window size
		for (int i = 0; i < -wyDelta; ++i)
		{
			if (cacheContiguous_->length() <= wy) break;
			if ((cacheContiguous_->back(0)->chunkList_->length() == 0) && (cy_ > 0)) {
				cacheContiguous_->popBack();
				--cy_;
			}
			else
				break;
		}
		
		// append empty lines, when expanding window size
		while (cacheContiguous_->length() < wy) {
			cacheContiguous_->pushBack(new Line(document()->tabWidth()));
			++cy_;
		}
	}
	
	if ((wy2 != wy) || (wx2 != wx)) {
		/*if (document() == documentContiguous_)
			setWindowSize(wy, renderCachingMaxLineLength());
		else*/
			setWindowSize(wy, wx);
	}
	
	updateVerticalScrollRange();
	if (atEnd) {
		if (verticalScrollBar()->value() != verticalScrollBar()->maximum())
			verticalScrollBar()->setValue(verticalScrollBar()->maximum());
	}
	
	limitCursor();
}

void QxVideoTerminal::keyPressEvent(QKeyEvent* event)
{
	#if PACU_IM_WORKAROUND
	if (ignoreNextKeyPressEvent_) {
		ignoreNextKeyPressEvent_ = false;
		return;
	}
	#endif
	
	struct termios tio;
	if (::tcgetattr(shell_->rawInput()->fd(), &tio) == -1) {
		qDebug() << "QxVideoTerminal::keyPressEvent(): tcgetattr() failed";
		mem::clr(&tio, sizeof(tio));
	}
	#if 0
	{
		debug("bool(tio.c_lflag & ICANON) = %%\n", bool(tio.c_lflag & ICANON));
		debug("bool(tio.c_lflag & IEXTEN) = %%\n", bool(tio.c_lflag & IEXTEN));
		debug("int(tio.c_cc[VERASE]) = %%\n", int(tio.c_cc[VERASE]));
		debug("int(tio.c_cc[VKILL]) = %%\n", int(tio.c_cc[VKILL]));
		debug("int(tio.c_cc[VLNEXT]) = %%\n", int(tio.c_cc[VLNEXT]));
		// debug("int(tio.c_cc[VWERASE]) = %%\n", int(tio.c_cc[VWERASE));
		/*print("bool(tio.c_lflag & ERASE) = %%\n", bool(tio.c_lflag & ERASE));
		print("bool(tio.c_lflag & KILL) = %%\n", bool(tio.c_lflag & KILL));*/
		/*debug("bool(tio.c_lflag & WERASE) = %%\n", bool(tio.c_lflag & WERASE));
		debug("bool(tio.c_lflag & REPRINT) = %%\n", bool(tio.c_lflag & REPRINT));
		debug("bool(tio.c_lflag & LNEXT) = %%\n", bool(tio.c_lflag & LNEXT));*/
		debug("-------------\n");
	}
	#endif
	
	int key = event->key();
	// bool shift = ((event->modifiers() & Qt::ShiftModifier) != 0);
	bool alt = (event->modifiers() & Qt::AltModifier) != 0;
	bool control = ((event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) != 0);
	QByteArray ba = event->text().toUtf8();
	
	// workaround HACK
	if (alt && (key == Qt::Key_Return)) {
		openLink();
		return;
	}
	
	if (control && (key == Qt::Key_R)) {
		ba.resize(1);
		ba[0] = CREPRINT;
	}
	else if (control && (key == Qt::Key_S)) {
		ba.resize(1);
		ba[0] = CSTOP;
	}
	else if (control && (key == Qt::Key_J)) {
		ba.resize(1);
		ba[0] = '\033';
	}
	
	#ifdef Q_WS_MAC
	// usability HACK
	if ((event->modifiers() & Qt::MetaModifier) != 0) {
		if (key == Qt::Key_A) {
			control = false;
			key = Qt::Key_Home;
		}
		else if (key == Qt::Key_E) {
			control = false;
			key = Qt::Key_End;
		}
	}
	#endif
	
	int mod = 1 + 1 * ((event->modifiers() & Qt::ShiftModifier) != 0)
	            + 2 * ((event->modifiers() & Qt::AltModifier) != 0)
	            + 4 * control;
	
	if ( (key == Qt::Key_Up) ||
	     (key == Qt::Key_Down) ||
	     (key == Qt::Key_Right) ||
	     (key == Qt::Key_Left) ||
	     ((key == Qt::Key_Home) && (!altKeypadMode_)) ||
	     ((key == Qt::Key_End) && (!altKeypadMode_)) )
	{
		char cmd = 0;
		
		if (control && (key == Qt::Key_Left)) { cmd = 'H'; mod -= 4; }
		else if (control && (key == Qt::Key_Right)) { cmd = 'F'; mod -= 4; }
		else if (key == Qt::Key_Up) cmd = 'A';
		else if (key == Qt::Key_Down) cmd = 'B';
		else if (key == Qt::Key_Right) cmd = 'C';
		else if (key == Qt::Key_Left) cmd = 'D';
		else if (key == Qt::Key_Home) cmd = 'H';
		else if (key == Qt::Key_End) cmd = 'F';
		
		if (mod != 1) {
			ba = "\033[1;mc";
			ba[ba.size()-2] = '0' + mod;
		}
		else {
			ba = "\033[c";
			if (applicationMode_)
				ba[1] = 'O';
		}
		ba[ba.size()-1] = cmd;
	}
	else if (key == Qt::Key_Home) ba = "\033[1~";
	else if (key == Qt::Key_Insert) ba = "\033[2~";
	else if (key == Qt::Key_Delete) ba = "\033[3~";
	else if (key == Qt::Key_End) ba = "\033[4~";
	else if (key == Qt::Key_PageUp) ba = "\033[5~";
	else if (key == Qt::Key_PageDown) ba = "\033[6~";
	else if (key == Qt::Key_Backspace) {
		if (tio.c_lflag & ICANON) {
			ba.resize(1);
			ba[0] = tio.c_cc[VERASE];
		}
		else
			ba = "\10";
	}
	else if (key == Qt::Key_F1)  ba = "\033[11~";
	else if (key == Qt::Key_F2)  ba = "\033[12~";
	else if (key == Qt::Key_F3)  ba = "\033[13~";
	else if (key == Qt::Key_F4)  ba = "\033[14~";
	else if (key == Qt::Key_F5)  ba = "\033[15~";
	else if (key == Qt::Key_F6)  ba = "\033[17~";
	else if (key == Qt::Key_F7)  ba = "\033[18~";
	else if (key == Qt::Key_F8)  ba = "\033[19~";
	else if (key == Qt::Key_F9)  ba = "\033[20~";
	else if (key == Qt::Key_F10) ba = "\033[21~";
	else if (key == Qt::Key_F11) ba = "\033[23~";
	else if (key == Qt::Key_F12) ba = "\033[24~";
	else if (key == Qt::Key_Escape) ba = "\033\033";
	
	if (ba.length() != 0) {
		// DEBUG
		/*for (int i = 0; i < ba.length(); ++i)
			ftl::print("(%dec%)", int(ba[i]));
		ftl::print("\n");*/
	
		try {
			shell_->rawInput()->write(ba.data(), ba.length());
		}
		catch (...) {
			// HACK, missing error handling
		}
		if ((!tinyDocScrollRange()) || (cy_ == 0)) // uncertain usability HACK
			if (verticalScrollBar()->value() != verticalScrollBar()->maximum())
				verticalScrollBar()->setValue(verticalScrollBar()->maximum());
		
		// for security reasons
		for (int i = 0, n = ba.length(); i < n; ++i)
			ba[i] = 0;
		
		if (hasSelection()) {
			unselect();
			update();
		}
	}
}

void QxVideoTerminal::forwardMouseEvent(QMouseEvent* event, int ty, int tx, bool moveEvent, bool mouseRelease)
{
	if ((ty == -1) || (tx == -1)) return;
	
	ty = cache()->length() - 1 - ty;
	
	if ((ty < windowLines()) && ((0 <= ty) && (ty < 224)) && (0 <= tx) && (tx < 224))
	{
		
		uint8_t cb = 0x20 + 0 * ((event->button() == Qt::LeftButton) || mouseRelease)
		                  + 1 * ((event->button() == Qt::MidButton) || mouseRelease)
		                  + 2 * ((event->button() == Qt::RightButton) || mouseRelease)
		                  + 4 * ((event->modifiers() & Qt::ShiftModifier) != 0)
		                  + 8 * ((event->modifiers() & Qt::AltModifier) != 0)
		                  + 16 * (((event->modifiers() & Qt::ControlModifier) != 0) || ((event->modifiers() & Qt::MetaModifier) != 0))
		                  + 32 * moveEvent;
		uint8_t cy = 0x20 + windowLines() - ty;
		uint8_t cx = 0x20 + tx + 1;
		uint8_t seq[6];
		seq[0] = 033;
		seq[1] = '[';
		seq[2] = 'M';
		seq[3] = cb;
		seq[4] = cx;
		seq[5] = cy;
		shell_->rawInput()->write(seq, 6);
	}
}

void QxVideoTerminal::mousePressEvent(QMouseEvent* event)
{
	if (mouseTracking_) {
		int ty, tx;
		mapMouseToTextPos(event->pos().x(), event->pos().y(), &ty, &tx);
		forwardMouseEvent(event, ty, tx, false, false);
	}
	else if (event->button() == Qt::MidButton) {
		if (hasSelection()) copy();
		paste();
	}
	else
		View::mousePressEvent(event);
}

void QxVideoTerminal::mouseMoveEvent(QMouseEvent* event)
{
	if (!mouseTracking_)
		View::mouseMoveEvent(event);
	
	if (mouseMotionTracking_ && (event->buttons() != 0)) {
		int ty0, tx0;
		int ty1, tx1;
		mapMouseToTextPos(mousePosSaved_.x(), mousePosSaved_.y(), &ty0, &tx0);
		mapMouseToTextPos(event->pos().x(), event->pos().y(), &ty1, &tx1);
		mousePosSaved_ = event->pos();
		if ((ty0 != ty1) || (tx0 != tx1))
			forwardMouseEvent(event, ty1, tx1, true, false);
	}
}

void QxVideoTerminal::mouseReleaseEvent(QMouseEvent* event)
{
	if (mouseTracking_) {
		int ty, tx;
		mapMouseToTextPos(event->pos().x(), event->pos().y(), &ty, &tx);
		forwardMouseEvent(event, ty, tx, false, true);
	}
	else
		View::mouseReleaseEvent(event);
}

void QxVideoTerminal::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (mouseTracking_)
	{
		mousePressEvent(event);
		mouseReleaseEvent(event);
	}
	else
		View::mouseDoubleClickEvent(event);
}

void QxVideoTerminal::wheelEvent(QWheelEvent* event)
{
	int steps = stepsFromWheelEvent(event);
	
	if (document() == documentFullscreen_) {
		char seq[3];
		seq[0] = 033;
		if (applicationMode_)
			seq[1] = 'O';
		else
			seq[1] = '[';
		if (steps > 0) {
			seq[2] = 'A';
		} else {
			seq[2] = 'B';
			steps = -steps;
		}
		if (hasSelection()) {
			unselect();
			update();
		}
		for (int i = 0; i < steps; ++i)
			shell_->rawInput()->write(seq, 3);
	}
	else
		View::wheelEvent(event);
}

void QxVideoTerminal::focusInEvent(QFocusEvent* event)
{
	update();
	struct termios tio;
	if (::tcgetattr(shell_->rawInput()->fd(), &tio) == -1) {
		qDebug() << "QxVideoTerminal::focusInEvent(): tcgetattr() failed";
		mem::clr(&tio, sizeof(tio));
	}
	else {
		Qt::InputMethodHints hints = inputMethodHints();
		if ((tio.c_lflag & ECHO) == 0)
			hints |= Qt::ImhHiddenText;
		else
			hints &= ~Qt::ImhHiddenText;
		setInputMethodHints(hints);
	}
	View::focusInEvent(event);
}

void QxVideoTerminal::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
		event->acceptProposedAction();
}

void QxVideoTerminal::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat("text/uri-list")) {
		QList<QUrl> urls = event->mimeData()->urls();
		for (int i = 0; i < urls.count(); ++i) {
			if (urls.at(i).isValid()) {
				QByteArray bytes = urls.at(i).toString().toUtf8();
				int cutOff = (urls.at(i).scheme() == "file") ? strlen("file://") : 0;
				shell()->rawInput()->write(bytes.constData() + cutOff, bytes.size() - cutOff);
				if (i != urls.count() - 1)
					shell()->rawInput()->write(" \\\n", 3);
			}
		}
		if (!isActiveWindow())
			activateWindow();
		setFocus();
	}
}

void QxVideoTerminal::clearScreen()
{
	autoOpenNextLink_ = false;
	resetLinkRange();
	cache()->clear();
	for (int i = 0; i < windowLines(); ++i)
		cache()->pushBack(new Line(document()->tabWidth()));
	textBuffer_->clear();
	sequenceType_ = UndefType;
	update();
}

void QxVideoTerminal::shellClear()
{
	shell_->rawInput()->write("clear\n", 6);
}

void QxVideoTerminal::inputMethodEvent(QInputMethodEvent* event)
{
	highlighterYield();
	
	bool beginInput = (event->preeditString() != preeditString_);
	bool finishInput = (!event->commitString().isEmpty()) || (event->replacementLength() > 0);
	bool modifying = (beginInput && hasSelection()) || finishInput;
	
	if (!preeditString_.isEmpty()) {
		// document()->setHistoryEnabled(false);
		// document()->setEmissionEnabled(false);
		document()->popSpan(-cy_ - 1, cx_, cx_ + preeditString_.length());
		// document()->setHistoryEnabled(true);
		// document()->setEmissionEnabled(true);
	}
	
	if (modifying) {
		// beginEdit();
		// document()->begin(saveState());
		deleteSelection();
	}
	
	for (int i = 0, n = event->attributes().size(); i < n; ++i)
	{
		const QInputMethodEvent::Attribute& a = event->attributes().at(i);
		if (a.type == QInputMethodEvent::Selection)
			select(-cy_ - 1, cx_ + a.start, -cy_ - 1, cx_ + a.start + a.length);
		else if (a.type == QInputMethodEvent::Cursor)
			preeditCursor_ = a.start * (a.length > 0); // quick HACK, unclear semantics
	}
	
	if (finishInput) {
		cx_ += event->replacementStart();
		if (event->replacementLength() > 0)
			document()->popSpan(-cy_ - 1, cx_, cx_ + event->replacementLength());
		if (!event->commitString().isEmpty()) {
			QByteArray utf8 =  event->commitString().toUtf8();
			shell_->rawInput()->write(utf8.data(), utf8.size());
			// document()->pushSpan(-cy_ - 1, cx_, &cx_, event->commitString());
		}
		preeditString_ = QString();
		preeditCursor_ = 0;
		#if PACU_IM_WORKAROUND
		ignoreNextKeyPressEvent_ = !event->commitString().isEmpty();
		#endif
	}
	else {
		preeditString_ = event->preeditString();
		if (!preeditString_.isEmpty()) {
			// document()->setHistoryEnabled(false);
			// document()->setEmissionEnabled(false);
			preeditStyle_ = new Style(colors()->foregroundColor_, colors()->matchColor_);
			document()->pushSpan(-cy_ - 1, cx_, 0, preeditString_, preeditStyle_);
			// document()->setHistoryEnabled(true);
			// document()->setEmissionEnabled(true);
		}
		update();
	}
	
	if (modifying)
		update();
	/*if (modifying) {
		document()->end(saveState());
		endEdit();
	}*/
	
	event->accept();
	
	highlighterResume();
}

} // namespace pacu
