#include <QDesktopServices>
#include <QSessionManager>
// #include <QSvgGenerator>
#include <ftl/streams>
#include <ftl/ByteDecoder.hpp>
#include <ftl/Utf8Decoder.hpp>
#include <ftl/Process.hpp>
#include <ftl/Semaphore.hpp>
#include <ftl/events>
#include <pte/Edit.hpp>
#include <pte/Context.hpp>
#include <pte/CharcoalDirectory.hpp>
#include <pte/CharcoalException.hpp>
#include <pte/LanguageManager.hpp>
#include <pte/LanguageLayer.hpp>
#include <pte/LanguageStack.hpp>
#include <pte/ThemeManager.hpp>
#include <pte/Highlighter.hpp>
#include <codetips/WordPredictor.hpp>
#include "QxApplication.hpp"
#include "QxCustomStyle.hpp"
#include "QxDialog.hpp"
#include "QxActivationDialog.hpp"
#include "QxAboutDialog.hpp"
#include "QxLicenseDialog.hpp"
#include "QxCurrentDirDialog.hpp"
#include "QxFileSystemWatcher.hpp"
#include "QxFileBrowser.hpp"
#include "QxCodeBrowser.hpp"
#include "QxFileSearchEdit.hpp"
#include "QxVideoTerminal.hpp"
#include "QxTerminalStack.hpp"
#include "QxPreferences.hpp"
#include "QxStatusBar.hpp"
#include "QxFindText.hpp"
#include "QxTabWidget.hpp"
#include "QxTabBar.hpp"
#include "QxWidgetStack.hpp"
#include "QxSplitter.hpp"
#include "QxActivation.hpp"
#include "SignalAdapter.hpp"
#include "InterpositionProxy.hpp"

#include "QxMainWindow.hpp"

#ifdef QT_MAC_USE_COCOA
// from qapplication_mac.mm
void qt_mac_set_app_icon(const QPixmap &pixmap);
#endif

namespace pacu
{

QxMainWindow::QxMainWindow()
	: QMainWindow(0, Qt::Window|Qt::WindowTitleHint|Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint|Qt::CustomizeWindowHint),
	  appLaunchTime_(true),
	  firstTimeActive_(true),
	  tabDragging_(false),
	  interpositionProxy_(new InterpositionProxy(this))
{
	try {
		charcoalDirectory_ = new CharcoalDirectory;
		languageManager_ = charcoalDirectory_->languageManager();
	}
	catch (CharcoalException& ex) {
		print("%%\n", ex.what());
		#ifdef Q_WS_X11
		#ifdef NDEBUG
		if (rawInput()->isTeletype())
		{
			QMessageBox* critical = new QMessageBox(this);
			critical->setIcon(QMessageBox::Critical);
			critical->setText(tr("Failed to load the language awareness files (\"charcoal\" environ.)."));
			critical->setInformativeText(tr("See the exception message below."));
			critical->setDetailedText(ex.what());
			critical->setStandardButtons(QMessageBox::Ok);
			critical->exec();
			delete critical;
		}
		#endif
		#endif
		Process::exit(1);
	}

#ifdef Q_WS_MAC
	setUnifiedTitleAndToolBarOnMac(true);
	setAttribute(Qt::WA_MacOpaqueSizeGrip, false);
#endif // Q_WS_MAC

	setWindowTitle(qApp->applicationName());
	setAcceptDrops(true);
	// setMinimumSize(QSize(400, 300));

	connect(qApp, SIGNAL(commitDataRequest(QSessionManager&)), this, SLOT(commitData(QSessionManager&)));

	preferences_ = new QxPreferences(charcoalDirectory_, this);
	connect(preferences_, SIGNAL(commandTriggered(QxCommand*)), this, SLOT(commandTriggered(QxCommand*)));
	connect(preferences_, SIGNAL(commandsChanged(Ref<QxCommandsList>)), this, SLOT(updateCommands(Ref<QxCommandsList>)));

	fileSystemWatcher_ = new QxFileSystemWatcher(this);
	connect(fileSystemWatcher_, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChanged(const QString&)));

	createFileMenu();

	editMenuDummy_ = createEditMenu();
	editMenuAction_ = new QAction(tr("&Edit"), this);
	editMenuAction_->setMenu(editMenuDummy_);
	menuBar()->addAction(editMenuAction_);

	createViewMenu();
	createNavMenu();
	createCommandsMenu();
	createHelpMenu();

	setupWindowIcon();
	setupToolbar();
	setupWorkspace();
	updateLanguageMenu();

	restoreState();

#ifdef Q_WS_MAC
	qApp->installEventFilter(this); // surpress showing tool tips
	connect(qxApp, SIGNAL(fileOpen(QString)), this, SLOT(openFile(QString)));
	#ifndef NDEBUG
	// quick HACK for bug in the debugger;)
	class NoAction: public Action { void run() { qDebug() << "NoAction:run(): SIGHUB: " << ::getpid(); }; };
	signalEvent(SIGHUP)->pushBack(new NoAction); // HACK, ignore SIGHUP
	#endif // NDEBUG
#endif

	{
		SignalAdapter* adapter = new SignalAdapter;
		connect(adapter, SIGNAL(emitted()), this, SLOT(quit()), Qt::QueuedConnection);
		signalEvent(SIGINT)->pushBack(adapter);
	}
}

QxMainWindow::~QxMainWindow()
{
	// interpositionProxy_->shutdown();
}

void QxMainWindow::buyNow()
{
	return;
	QxActivationDialog* dialog = new QxActivationDialog(this);
	dialog->exec();
	if (activation()->timeLeft() <= 0) quit();
	{
		QSettings settings;
		preferences_->save(&settings);
	}
	delete dialog;
}

#ifndef QT_MAC_USE_COCOA
void QxMainWindow::reopen()
{
	if (!isVisible()) {
		restoreState();
		show();
	}
	if (isMinimized())
		showNormal();
	if (QApplication::activeWindow() != this)
		activateWindow();
}
#endif

void QxMainWindow::createFileMenu()
{
	QAction* fileNew = new QAction(tr("New"), this);
	QAction* fileOpen = new QAction(tr("Open"), this);
	QAction* fileOpenRecent = new QAction(tr("Open Recent"), this);
	QAction* cwdSet = new QAction(tr("Working Directory"), this);
	QAction* fileSave = new QAction(tr("Save"), this);
	QAction* fileSaveAs = new QAction(tr("Save As"), this);
	QAction* fileSaveAll = new QAction(tr("Save All"), this);
	QAction* fileGotoHere = new QAction(tr("Goto Here"), this);
	QAction* fileReload = new QAction(tr("Reload"), this);
	QAction* fileExportHtml = new QAction(tr("Export HTML"), this);
	QAction* filePrint = new QAction(tr("Print"), this);
	QAction* fileClose = new QAction(tr("Close"), this);
	QAction* fileCloseAll = new QAction(tr("Close All"), this);
	QAction* quit = new QAction(tr("Quit"), this);

	fileNew->setShortcut(tr("Ctrl+N"));
	fileOpen->setShortcut(tr("Ctrl+O"));
	cwdSet->setShortcut(tr("Ctrl+."));
	fileSave->setShortcut(tr("Ctrl+S"));
	fileSaveAs->setShortcut(tr("Ctrl+Shift+S"));
	fileSaveAll->setShortcut(tr("Ctrl+Alt+S"));
	fileGotoHere->setShortcut(tr("Ctrl+H"));
	fileReload->setShortcut(tr("Ctrl+R"));
	filePrint->setShortcut(tr("Ctrl+P"));
	fileClose->setShortcut(tr("Ctrl+W"));
	fileCloseAll->setShortcut(tr("Ctrl+Alt+W"));
	quit->setShortcut(tr("Ctrl+Q"));

	connect(fileNew, SIGNAL(triggered()), this, SLOT(newFile()));
	connect(fileOpen, SIGNAL(triggered()), this, SLOT(openFile()));
	connect(cwdSet, SIGNAL(triggered()), this, SLOT(cwdDialog()));
	connect(fileSave, SIGNAL(triggered()), this, SLOT(saveFile()));
	connect(fileSaveAs, SIGNAL(triggered()), this, SLOT(saveFileAs()));
	connect(fileSaveAll, SIGNAL(triggered()), this, SLOT(saveFileAll()));
	connect(fileGotoHere, SIGNAL(triggered()), this, SLOT(gotoHere()));
	connect(fileReload, SIGNAL(triggered()), this, SLOT(reloadFile()));
	connect(fileExportHtml, SIGNAL(triggered()), this, SLOT(exportToHtml()));
	connect(filePrint, SIGNAL(triggered()), this, SLOT(printFile()));
	connect(fileClose, SIGNAL(triggered()), this, SLOT(closeTab()));
	connect(fileCloseAll, SIGNAL(triggered()), this, SLOT(closeFileAll()));
	#ifdef Q_WS_MAC
	connect(quit, SIGNAL(triggered()), this, SLOT(quit()));
	#else
	connect(quit, SIGNAL(triggered()), this, SLOT(close()));
	#endif

	connect(fileSave, SIGNAL(triggered()), this, SLOT(updateCodeBrowser()));
	connect(fileSaveAs, SIGNAL(triggered()), this, SLOT(updateCodeBrowser()));
	connect(fileSaveAll, SIGNAL(triggered()), this, SLOT(updateCodeBrowser()));

	fileWidgetActions_.append(fileSave);
	fileWidgetActions_.append(fileSaveAs);
	fileWidgetActions_.append(fileReload);
	fileWidgetActions_.append(filePrint);
	for (int i = 0, n = fileWidgetActions_.count(); i < n; ++i)
		fileWidgetActions_.at(i)->setShortcutContext(Qt::WidgetShortcut);

	fileOpenRecent_ = fileOpenRecent;
	fileReload_ = fileReload;
	fileClose_ = fileClose;

	quit->setMenuRole(QAction::QuitRole);

	QMenu* menu = new QMenu(tr("&File"), this);
	menu->addAction(fileNew);
	menu->addAction(fileOpen);
	menu->addAction(fileOpenRecent);
	menu->addSeparator();
	menu->addAction(cwdSet);
	menu->addSeparator();
	menu->addAction(fileSave);
	menu->addAction(fileSaveAs);
	menu->addAction(fileSaveAll);
	menu->addSeparator();
	menu->addAction(fileGotoHere);
	menu->addAction(fileReload);
	menu->addAction(fileExportHtml);
	menu->addAction(filePrint);
	menu->addSeparator();
	menu->addAction(fileClose);
	menu->addAction(fileCloseAll);
	#ifndef Q_WS_MAC
	menu->addSeparator();
	#endif
	menu->addAction(quit);
	menuBar()->addMenu(menu);
}

QMenu* QxMainWindow::createEditMenu(Edit* edit)
{
	QWidget* parent = edit;
	if (!parent) parent = this;
	QMenu* menu = Edit::setupActions(parent);
	QAction* gotoLine = new QAction(tr("Goto Line"), parent);
	gotoLine->setShortcut(tr("Ctrl+G"));
	if (edit) {
		gotoLine->setShortcutContext(Qt::WidgetShortcut);
		edit->addAction(gotoLine);
		connect(gotoLine, SIGNAL(triggered()), this, SLOT(gotoLineDialog()));
	}
	menu->addSeparator();
	menu->addAction(gotoLine);
	if (!edit) {
		QList<QAction*> actions = menu->actions();
		for (int i = 0, n = actions.count(); i < n; ++i)
			actions.at(i)->setEnabled(false);
	}
	return menu;
}

void QxMainWindow::createViewMenu()
{
	#ifndef QT_MAC_USE_COCOA
	QAction* fullscreenMode = new QAction(tr("Fullscreen Mode"), this);
	fullscreenMode->setCheckable(true);
	#endif
	windowMaximized_ = new QAction(tr("Window Maximized"), this);
	windowMaximized_->setCheckable(true);
	toolbarShow_ = new QAction(tr("Toolbar Show"), this);
	toolbarShow_->setCheckable(true);
	QAction* fileBrowserShow = new QAction(tr("File Browser Show"), this);
	QAction* fileBrowserHide = new QAction(tr("File Browser Hide"), this);
	QAction* codeBrowserShow = new QAction(tr("Code Browser Show"), this);
	QAction* codeBrowserHide = new QAction(tr("Code Browser Hide"), this);
	QAction* editorShow = new QAction(tr("Editor Show"), this);
	QAction* editorHide = new QAction(tr("Editor Hide"), this);
	QAction* terminalShow = new QAction(tr("Terminal Show"), this);
	QAction* terminalHide = new QAction(tr("Terminal Hide"), this);
	QAction* findShow = new QAction(tr("Find/Replace Show"), this);
	QAction* findHide = new QAction(tr("Find/Replace Hide"), this);
	QAction* editorShowMaximized = new QAction(tr("Editor Maximize"), this);
	QAction* terminalShowMaximized = new QAction(tr("Terminal Maximize"), this);
	QAction* preferencesShow = new QAction(tr("Preferences"), this);

	#ifndef QT_MAC_USE_COCOA
	fullscreenMode->setShortcut(tr("Ctrl+Alt+M"));
	#endif
	windowMaximized_->setShortcut(tr("Ctrl+M"));
	toolbarShow_->setShortcut(tr("Ctrl+|"));
	fileBrowserShow->setShortcut(tr("Ctrl+B"));
	fileBrowserHide->setShortcut(tr("Ctrl+Shift+B"));
	codeBrowserShow->setShortcut(tr("Ctrl+I"));
	codeBrowserHide->setShortcut(tr("Ctrl+Shift+I"));
	editorShow->setShortcut(tr("Ctrl+E"));
	editorHide->setShortcut(tr("Ctrl+Shift+E"));
	#ifdef QT_MAC_USE_COCOA
	terminalShow->setShortcut(tr("Ctrl+\n"));
	terminalHide->setShortcut(tr("Ctrl+Shift+\n"));
	#else
	terminalShow->setShortcut(tr("Ctrl+RETURN"));
	terminalHide->setShortcut(tr("Ctrl+Shift+RETURN"));
	#endif
	findShow->setShortcut(tr("Ctrl+F"));
	findHide->setShortcut(tr("Ctrl+Shift+F"));
	editorShowMaximized->setShortcut(tr("Ctrl+Alt+E"));
	#ifdef QT_MAC_USE_COCOA
	terminalShowMaximized->setShortcut(tr("Ctrl+Alt+\n"));
	#else
	terminalShowMaximized->setShortcut(tr("Ctrl+Alt+RETURN"));
	#endif
	preferencesShow->setShortcut(tr("Ctrl+,"));

	/* fullscreenMode->setCheckable(true);
	windowMaximized_->setCheckable(true); */

	#ifndef QT_MAC_USE_COCOA
	connect(fullscreenMode, SIGNAL(triggered()), this, SLOT(toggleFullscreenMode()));
	#endif
	connect(windowMaximized_, SIGNAL(triggered()), this, SLOT(toggleShowMaximized()));
	connect(toolbarShow_, SIGNAL(triggered()), this, SLOT(toggleShowToolbar()));
	connect(fileBrowserShow, SIGNAL(triggered()), this, SLOT(showFileBrowser()));
	connect(fileBrowserHide, SIGNAL(triggered()), this, SLOT(hideFileBrowser()));
	connect(codeBrowserShow, SIGNAL(triggered()), this, SLOT(showCodeBrowser()));
	connect(codeBrowserHide, SIGNAL(triggered()), this, SLOT(hideCodeBrowser()));
	connect(editorShow, SIGNAL(triggered()), this, SLOT(showEditor()));
	connect(editorHide, SIGNAL(triggered()), this, SLOT(hideEditor()));
	connect(terminalShow, SIGNAL(triggered()), this, SLOT(showTerminal()));
	connect(terminalHide, SIGNAL(triggered()), this, SLOT(hideTerminal()));
	connect(findShow, SIGNAL(triggered()), this, SLOT(showFindText()));
	connect(findHide, SIGNAL(triggered()), this, SLOT(hideFindText()));
	connect(editorShowMaximized, SIGNAL(triggered()), this, SLOT(editorShowMaximized()));
	connect(terminalShowMaximized, SIGNAL(triggered()), this, SLOT(terminalShowMaximized()));
	#ifdef Q_WS_MAC
	connect(preferencesShow, SIGNAL(triggered()), preferences_, SLOT(exec()));
	#else
	connect(preferencesShow, SIGNAL(triggered()), preferences_, SLOT(show()));
	#endif

	preferencesShow->setMenuRole(QAction::PreferencesRole);

	QMenu* menu = new QMenu(tr("&View"), this);
	#ifndef QT_MAC_USE_COCOA
	menu->addAction(fullscreenMode);
	#endif
	menu->addAction(windowMaximized_);
	#ifndef QT_MAC_USE_COCOA
	menu->addAction(toolbarShow_);
	#endif
	menu->addSeparator();
	menu->addAction(fileBrowserShow);
	menu->addAction(fileBrowserHide);
	menu->addAction(codeBrowserShow);
	menu->addAction(codeBrowserHide);
	menu->addAction(editorShow);
	menu->addAction(editorHide);
	menu->addAction(terminalShow);
	menu->addAction(terminalHide);
	menu->addAction(findShow);
	menu->addAction(findHide);
	menu->addSeparator();
	menu->addAction(editorShowMaximized);
	menu->addAction(terminalShowMaximized);
	#ifndef Q_WS_MAC
	menu->addSeparator();
	#endif
	menu->addAction(preferencesShow);

	QList<QAction*> actions = menu->actions();
	for (int i = 0, n = actions.count(); i < n; ++i)
		actions.at(i)->setIconVisibleInMenu(false);

	connect(menu, SIGNAL(aboutToShow()), this, SLOT(updateViewMenu()));
	menuBar()->addMenu(menu);
}

void QxMainWindow::createNavMenu()
{
	QAction* fileOpenQuick = new QAction(tr("Quick File Open"), this);
	QAction* tabOpen = new QAction(tr("Open Tab"), this);
	QAction* tabClose = new QAction(tr("Close Tab"), this);
	QAction* tabPrevious = new QAction(tr("Previous Tab"), this);
	QAction* tabNext = new QAction(tr("Next Tab"), this);
	QAction* tabDragLeft = new QAction(tr("Drag Tab Left"), this);
	QAction* tabDragRight = new QAction(tr("Drag Tab Right"), this);
	QAction* linkOpen = new QAction(tr("Open Link"), this);
	QAction* linkFirst = new QAction(tr("First Link"), this);
	QAction* linkLast = new QAction(tr("Last Link"), this);
	QAction* linkPrevious = new QAction(tr("Previous Link"), this);
	QAction* linkNext = new QAction(tr("Next Link"), this);

	fileOpenQuick->setShortcut(tr("Ctrl+Alt+O"));
	tabOpen->setShortcut(tr("Ctrl+T"));
	tabClose->setShortcut(tr("Ctrl+Shift+T"));
	#ifdef Q_WS_X11
	tabPrevious->setShortcut(tr("Ctrl+Shift+TAB"));
	tabNext->setShortcut(tr("Ctrl+TAB"));
	tabDragLeft->setShortcut(tr("Meta+Alt+LEFT"));
	tabDragRight->setShortcut(tr("Meta+Alt+RIGHT"));
	#else
	tabPrevious->setShortcut(tr("Meta+Shift+TAB"));
	tabNext->setShortcut(tr("Meta+TAB"));
	tabDragLeft->setShortcut(tr("Meta+Alt+LEFT"));
	tabDragRight->setShortcut(tr("Meta+Alt+RIGHT"));
	#endif
	#ifdef QT_MAC_USE_COCOA
	linkOpen->setShortcut(tr("Alt+\n"));
	#else
	linkOpen->setShortcut(tr("Alt+RETURN"));
	#endif
	linkFirst->setShortcut(tr("Alt+HOME"));
	linkLast->setShortcut(tr("Alt+END"));
	linkPrevious->setShortcut(tr("Alt+PGUP"));
	linkNext->setShortcut(tr("Alt+PGDOWN"));

	connect(fileOpenQuick, SIGNAL(triggered()), this, SIGNAL(openFileQuick()));
	connect(tabOpen, SIGNAL(triggered()), this, SLOT(openTab()));
	connect(tabClose, SIGNAL(triggered()), this, SLOT(closeTab()));
	connect(tabPrevious, SIGNAL(triggered()), this, SLOT(previousTab()));
	connect(tabNext, SIGNAL(triggered()), this, SLOT(nextTab()));
	connect(tabDragLeft, SIGNAL(triggered()), this, SLOT(dragTabLeft()));
	connect(tabDragRight, SIGNAL(triggered()), this, SLOT(dragTabRight()));
	connect(linkOpen, SIGNAL(triggered()), this, SLOT(openLink()));
	connect(linkFirst, SIGNAL(triggered()), this, SLOT(firstLink()));
	connect(linkLast, SIGNAL(triggered()), this, SLOT(lastLink()));
	connect(linkPrevious, SIGNAL(triggered()), this, SLOT(previousLink()));
	connect(linkNext, SIGNAL(triggered()), this, SLOT(nextLink()));

	QMenu* menu = new QMenu(tr("Navi&gation"), this);
	menu->addAction(fileOpenQuick);
	menu->addAction(tabOpen);
	menu->addAction(tabClose);
	menu->addSeparator();
	menu->addAction(tabPrevious);
	menu->addAction(tabNext);
	menu->addAction(tabDragLeft);
	menu->addAction(tabDragRight);
	menu->addSeparator();
	menu->addAction(linkOpen);
	menu->addAction(linkFirst);
	menu->addAction(linkLast);
	menu->addAction(linkPrevious);
	menu->addAction(linkNext);
	menuBar()->addMenu(menu);
}

void QxMainWindow::createCommandsMenu()
{
	commandsSetup_ = 0;
	commandsMenuAction_ = new QAction(tr("Comman&ds"), this);
	commandsMenuAction_->setMenu(new QMenu(this));
	menuBar()->addAction(commandsMenuAction_);
}

void QxMainWindow::createHelpMenu()
{
	QAction* about = new QAction(
	#ifdef Q_WS_MAC
		tr("About %1").arg(qApp->applicationName()),
	#else
		tr("About"),
	#endif
		this
	);
	about->setMenuRole(QAction::ApplicationSpecificRole);
	connect(about, SIGNAL(triggered()), this, SLOT(about()));

	QAction* license = new QAction(tr("License"), this);
	// license->setMenuRole(QAction::ApplicationSpecificRole);
	connect(license, SIGNAL(triggered()), this, SLOT(license()));

	QAction* manual = new QAction(tr("Users's Manual"), this);
	connect(manual, SIGNAL(triggered()), this, SLOT(onlineManual()));

	QAction* activation = new QAction(tr("Activation..."), this);
	activation->setMenuRole(QAction::ApplicationSpecificRole);
	connect(activation, SIGNAL(triggered()), this, SLOT(openActivation()));

	/*QAction* screenshot = new QAction(tr("Screenshot"), this);
	connect(screenshot, SIGNAL(triggered()), this, SLOT(screenshot()));*/

	QMenu* menu = new QMenu(tr("&Help"), this);
	menu->addAction(about);
	menu->addAction(license);
	menu->addAction(manual);
	#ifdef Q_WS_X11
	menu->addSeparator();
	#endif
	menu->addAction(activation);
	// menu->addAction(screenshot);
	menuBar()->addMenu(menu);
}

void QxMainWindow::setupWindowIcon()
{
	#ifdef QT_MAC_USE_COCOA
		#include "logo/128x128/pacu.c"
		QPixmap pm;
		pm.loadFromData(pacu, pacuSize);
		qt_mac_set_app_icon(pm);
	#else
		QIcon icon;
		{
			#include "logo/16x16/pacu.c"
			QPixmap pm;
			pm.loadFromData(pacu, pacuSize);
			icon.addPixmap(pm);
		}
		{
			#include "logo/32x32/pacu.c"
			QPixmap pm;
			pm.loadFromData(pacu, pacuSize);
			icon.addPixmap(pm);
		}
		{
			#include "logo/64x64/pacu.c"
			QPixmap pm;
			pm.loadFromData(pacu, pacuSize);
			icon.addPixmap(pm);
		}
		{
			#include "logo/128x128/pacu.c"
			QPixmap pm;
			pm.loadFromData(pacu, pacuSize);
			icon.addPixmap(pm);
		}
		{
			#include "logo/256x256/pacu.c"
			QPixmap pm;
			pm.loadFromData(pacu, pacuSize);
			icon.addPixmap(pm);
		}
		{
			#include "logo/512x512/pacu.c"
			QPixmap pm;
			pm.loadFromData(pacu, pacuSize);
			icon.addPixmap(pm);
		}
		qApp->setWindowIcon(icon);
	#endif
}

void QxMainWindow::setupToolbar()
{
	QAction* fileBrowserShow = new QAction(tr("Files"), this);
	// QAction* codeBrowserShow = new QAction(tr("Tags"), this);
	QAction* terminalShow = new QAction(tr("Term."), this);
	QAction* preferencesShow = new QAction(tr("Setup"), this);
	QAction* scriptAdd = new QAction(tr("Cmds."), this);

	connect(fileBrowserShow, SIGNAL(triggered()), this, SLOT(toggleFileBrowser()));
	// connect(codeBrowserShow, SIGNAL(triggered()), this, SLOT(toggleCodeBrowser()));
	connect(terminalShow, SIGNAL(triggered()), this, SLOT(toggleTerminal()));
	connect(preferencesShow, SIGNAL(triggered()), preferences_, SLOT(showOrHide()/*showTabEditor()*/));
	connect(scriptAdd, SIGNAL(triggered()), preferences_, SLOT(showTabCommands()));

	{
		#include "sky/24x24/filebrowser.c"
		QPixmap icon;
		icon.loadFromData(filebrowser, filebrowserSize);
		fileBrowserShow->setIcon(icon);
	}
	/*{
		#include "sky/24x24/codebrowser.c"
		QPixmap icon;
		icon.loadFromData(codebrowser, codebrowserSize);
		codeBrowserShow->setIcon(icon);
	}*/
	{
		#include "logo/24x24/tlight2.c"
		QPixmap icon;
		icon.loadFromData(tlight2, tlight2Size);
		terminalShow->setIcon(icon);
	}
	{
		#include "sky/24x24/preferences.c"
		QPixmap icon;
		icon.loadFromData(preferences, preferencesSize);
		preferencesShow->setIcon(icon);
	}
	{
		#include "sky/24x24/scriptadd.c"
		QPixmap icon;
		icon.loadFromData(scriptadd, scriptaddSize);
		scriptAdd->setIcon(icon);
	}
	/*{
		#include "sky/32x32/search.c"
		QPixmap icon;
		icon.loadFromData(search, searchSize);
		findShow->setIcon(icon);
	}*/

	toolbar_ = addToolBar(tr("Toolbar"));
	toolbar_->setObjectName("toolBar");
	#ifdef Q_WS_MAC
	toolbar_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	#else
	toolbar_->setToolButtonStyle(Qt::ToolButtonFollowStyle);
	#endif
	toolbar_->setFloatable(false);
	toolbar_->setMovable(false);
	toolbar_->setIconSize(QSize(24, 24));
	toolbar_->addAction(fileBrowserShow);
	// toolbar_->addAction(codeBrowserShow);
	toolbar_->addAction(terminalShow);
	toolbar_->addAction(preferencesShow);
	toolbar_->addAction(scriptAdd);
	//toolbar_->addAction("...");

	firstCommand_ = toolbar_->addAction("First Command");
	firstCommand_->setVisible(false);

	QWidget* spacer = styleManager()->space(-1, 1, toolbar_);
	// spacer->setSizePolicy(QSizePolicy::Expanding, spacer->sizePolicy().verticalPolicy());
	toolbar_->addWidget(spacer);
	// toolbar_->addWidget(styleManager()->space(5, 1, this));

	fileSearchEdit_ = new QxFileSearchEdit(this, Qt::AlignRight);
	connect(fileSearchEdit_, SIGNAL(openFile(QString)), this, SLOT(openFile(QString)));
	connect(fileSearchEdit_, SIGNAL(leadingPressed()), this, SLOT(cwdDialog()));
	connect(this, SIGNAL(openFileQuick()), fileSearchEdit_, SLOT(setFocus()));
	/*{
		QWidgetAction* action = new QWidgetAction(this);
		action->setText(tr("Quick file open"));
		action->setIconText(tr("Quick file open"));
		action->setDefaultWidget(fileSearchEdit_);
		toolbar_->addAction(action);
	}*/
	toolbar_->addWidget(fileSearchEdit_);

	#ifdef Q_WS_MAC
	#ifdef QT_MAC_USE_COCOA
	toolbar_->addWidget(styleManager()->space(1, 1, this));
	#else
	toolbar_->addWidget(styleManager()->space(5, 1, this));
	#endif
	#endif
}

void QxMainWindow::setupWorkspace()
{
	{
		fileBrowserDock_ = new QDockWidget(tr("File Browser"), this);
		fileBrowserDock_->setObjectName("fileBrowserDock");
		fileBrowserDock_->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
		// fileBrowserDock_->setFeatures(QDockWidget::AllDockWidgetFeatures);

		fileBrowser_ = new QxFileBrowser(fileBrowserDock_);
		connect(fileBrowser_, SIGNAL(openFile(QString)), this, SLOT(openFile(QString)));
		connect(fileBrowser_, SIGNAL(fileRenamed(QString, QString)), this, SLOT(fileRenamed(QString, QString)));
		connect(fileBrowser_, SIGNAL(escape()), this, SLOT(hideFileBrowser()));
		connect(fileBrowser_, SIGNAL(cwdChanged(QString)), fileSearchEdit_, SLOT(setCurrentDir(QString)));

		addDockWidget(Qt::LeftDockWidgetArea, fileBrowserDock_);
	}
	{
		codeBrowserDock_ = new QDockWidget(tr("Code Browser"), this);
		codeBrowserDock_->setObjectName("codeBrowserDock");
		codeBrowserDock_->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);

		codeBrowser_ = new QxCodeBrowser(codeBrowserDock_);
		connect(codeBrowser_, SIGNAL(gotoLine(int)), this, SLOT(gotoLine(int)));

		addDockWidget(Qt::RightDockWidgetArea, codeBrowserDock_);
	}

	tabWidget_ = new QxTabWidget(this);
	tabWidget_->tabBar()->setAcceptUriDrops(true);
	tabWidget_->tabBar()->setSemanticType("document");
	{
		QMenu* menu = new QMenu(this);
		menu->addAction(fileReload_);
		menu->addAction(fileClose_);
		tabWidget_->tabBar()->setContextMenu(menu);
	}
	connect(tabWidget_->tabBar(), SIGNAL(openRequest(int, QUrl)), this, SLOT(openFile(int, QUrl)));
	connect(tabWidget_->tabBar(), SIGNAL(closeRequest(int)), this, SLOT(closeFile(int)));
	connect(tabWidget_->tabBar(), SIGNAL(lastTabClosed()), this, SLOT(lastTabClosed()));
	connect(
		tabWidget_->tabBar(), SIGNAL(changeActiveWidget(QWidget*, QWidget*)),
		this, SLOT(changeActiveEdit(QWidget*, QWidget*))
	);

	terminalStack_ = new QxTerminalStack(preferences_, this);
	connect(terminalStack_, SIGNAL(gotoLinkRequest(Ref<Link, Owner>)), this, SLOT(gotoLink(Ref<Link, Owner>)));
	connect(terminalStack_, SIGNAL(openFile(QString)), this, SLOT(openFile(QString)));

	findText_ = new QxFindText(preferences_, this);
	connect(preferences_, SIGNAL(editorMetricsChanged(Ref<ViewMetrics>)), findText_, SLOT(setViewMetrics(Ref<ViewMetrics>)));
	connect(preferences_, SIGNAL(terminalPaletteChanged()), findText_, SLOT(terminalPaletteChanged()));
	connect(charcoalDirectory_->themeManager(), SIGNAL(viewColorsChanged(Ref<ViewColors>)), findText_, SLOT(setViewColors(Ref<ViewColors>)));
	connect(findText_, SIGNAL(gotoMatch(Ref<Document>, int, int, int)), this, SLOT(gotoMatch(Ref<Document>, int, int, int)));
	connect(findText_, SIGNAL(showMessage(QString)), this, SLOT(matchStatusMessage(QString)));
	connect(findText_, SIGNAL(openFile(Ref<QxFileOpenRequest, Owner>)), this, SLOT(openFile(Ref<QxFileOpenRequest, Owner>)));

	connect(terminalStack_, SIGNAL(activeTerminalChanged(QxVideoTerminal*)), findText_, SLOT(setVideoTerminal(QxVideoTerminal*)));
	findText_->setVideoTerminal(terminalStack_->activeTerminal());

	statusBar_ = new QxStatusBar(this);

	terminalSwitch_ = new QxControl(this, new QxVisual(styleManager()->style("workspaceSwitch")));
	terminalSwitch_->setMode(QxControl::ToggleMode);
	terminalSwitch_->visual()->setText(tr("Terminal"));
	connect(terminalSwitch_, SIGNAL(toggled(bool)), this, SLOT(switchTerminal(bool)));

	findTextSwitch_ = new QxControl(this, new QxVisual(styleManager()->style("workspaceSwitch")));
	findTextSwitch_->setMode(QxControl::ToggleMode);
	findTextSwitch_->visual()->setText(tr("Find / Replace"));
	connect(findTextSwitch_, SIGNAL(toggled(bool)), this, SLOT(switchFindText(bool)));
	connect(findText_, SIGNAL(escape()), findTextSwitch_, SLOT(press()));

	gotoLineButton_ = new QxControl(this, new QxVisual(styleManager()->style("gotoLineButton")));
	gotoLineButton_->setMode(QxControl::TouchMode);
	connect(gotoLineButton_, SIGNAL(pressed()), this, SLOT(gotoLineDialog()));

	languageButton_ = new QxControl(this, new QxVisual(styleManager()->style("languageButton")));
	languageButton_->setMode(QxControl::MenuMode);
	languageButton_->setMenuPopupMode(QxControl::PreferUpsideMenuPopup);
	languageButton_->visual()->setText(languageManager_->stackByIndex(languageManager_->numStacks() - 1)->displayName());

	// architecture HACK
	// restore file browser state early to get the shell startup in previous cwd
	QSettings settings;
	settings.beginGroup("session");
	fileBrowser_->restoreState(&settings);
	settings.endGroup();

	toolStack_ = new QWidget(this);
	toolStackLayout_ = new QStackedLayout;
	toolStackLayout_->addWidget(terminalStack_);
	toolStackLayout_->addWidget(findText_);
	toolStack_->setLayout(toolStackLayout_);

	workspaceSplitter_ = new QxSplitter(Qt::Vertical, this);
	workspaceSplitter_->setHandle(1, new QxControl(this, new QxVisual(styleManager()->style("workspaceSplitter"))));
	workspaceSplitter_->setHandleWidth(styleManager()->constant("workspaceSplitterWidth"));
	workspaceSplitter_->addWidget(tabWidget_);
	workspaceSplitter_->addWidget(toolStack_);
	workspaceSplitter_->setStretchFactor(0, 8);
	workspaceSplitter_->setStretchFactor(1, 1);

	QWidget* workspace = new QWidget(this);
	{
		QVBoxLayout* col = new QVBoxLayout;
		col->setSpacing(0);
		col->setMargin(0);
		col->addWidget(workspaceSplitter_);
		col->addWidget(styleManager()->hl(this));
		{
			QHBoxLayout* row = new QHBoxLayout;
			row->addWidget(terminalSwitch_);
			row->addWidget(styleManager()->vl(this));
			row->addWidget(findTextSwitch_);
			row->addWidget(styleManager()->vl(this));
			row->addWidget(statusBar_);
			row->addWidget(styleManager()->vl(this));
			row->addWidget(languageButton_);
			row->addWidget(styleManager()->vl(this));
			row->addWidget(gotoLineButton_);
			row->addWidget(new QxControl(this, new QxVisual(styleManager()->style("resizeHandleGap"))));
			col->addLayout(row);
		}
		workspace->setLayout(col);
	}

	setCentralWidget(workspace);
}

void QxMainWindow::restoreState()
{
	bool firstTime = false;

	QSettings settings;

	activation()->restoreState(&settings);

	settings.beginGroup("session");

	findText_->restoreState(&settings);

	#ifdef Q_WS_MAC
	// if (!settings.value("mainWindow/toolbar/visible", true).toBool())
	// toolbar_->setVisible(false);
	#endif
	/*#ifdef Q_WS_MAC
	if (settings.contains("mainWindow/state"))
		QMainWindow::restoreState(settings.value("mainWindow/state").toByteArray());
	#endif*/
	#ifdef Q_WS_X11
	if (!appLaunchTime_)
	#endif
	if (settings.contains("mainWindow/pos"))
		move(settings.value("mainWindow/pos").toPoint());
	if (settings.contains("mainWindow/size"))
		resize(settings.value("mainWindow/size").toSize());
	#ifdef Q_WS_MAC
	if (appLaunchTime_)
		if (settings.contains("mainWindow/wasMaximized"))
			if (settings.value("mainWindow/wasMaximized").toBool())
				QTimer::singleShot(0, this, SLOT(showMaximized()));
	#endif
	appLaunchTime_ = false;

	if (settings.contains("mainWindow/state")) {
		QMainWindow::restoreState(settings.value("mainWindow/state").toByteArray());
		if (!settings.contains("mainWindow/codeBrowserHack"))
			codeBrowserDock_->setVisible(false);
	}
	else
		firstTime = true;
	if (settings.contains("workspaceSplitter"))
		workspaceSplitter_->restoreState(settings.value("workspaceSplitter").toByteArray());

	settings.endGroup();

	preferences_->restore(&settings);

	settings.beginGroup("session");

	// architecture HACK
	// file browser state could be restored here
	// (but is restored earlier to get the shell startup in previous cwd)

	if (settings.contains("toolStack/visible")) {
		toolStack_->setVisible(settings.value("toolStack/visible").toBool());
	}
	if (settings.contains("terminal/visible")) {
		if (settings.value("terminal/visible").toBool()) {
			terminalSwitch_->toggle(true);
		}
	}
	if (settings.contains("findText/visible")) {
		if (settings.value("findText/visible").toBool()) {
			findTextSwitch_->toggle(true);
		}
	}

	if (settings.contains("recentFiles")) {
		recentFiles_ = settings.value("recentFiles").toStringList();
	}
	updateRecentFilesMenu();

	if (settings.contains("tabWidget")) {
		QStringList files = settings.value("tabWidget", QStringList()).toStringList();
		QStringList failedToOpen;
		for (int i = 0; i < files.count(); ++i) {
			bool failed;
			openFile(files.at(i), &failed);
			if (failed)
				failedToOpen << files.at(i);
		}
		if (failedToOpen.count() > 0) {
			QMessageBox* critical = new QMessageBox(this);
			critical->setIcon(QMessageBox::Critical);
			critical->setText(tr("Failed to open one or more files from your last session."));
			critical->setInformativeText(tr("Probably the files have been moved or deleted."));
			critical->setDetailedText(failedToOpen.join("\n"));
			critical->setStandardButtons(QMessageBox::Ok);
			#ifdef Q_WS_MAC
			QList<QFrame*> frames = critical->findChildren<QFrame*>();
			for (int i = 0; i < frames.count(); ++i) {
				QFrame* frame = frames.at(i);
				if (frame->frameShape() == QFrame::HLine)
					frame->setFrameShape(QFrame::NoFrame);
			}
			#endif
			critical->exec();
			delete critical;
		}
	}
	if (settings.contains("tabWidgetIndexActive"))
		tabWidget_->tabBar()->activate(settings.value("tabWidgetIndexActive").toInt());

	/*for (int i = 0; i < tabWidget_->tabBar()->tabCount(); ++i) {
		QString key = QString("editFocus_%1").arg(i);
		if (settings.contains(key)) {
			if (settings.value(key).toBool()) {
				textEdit(i)->setFocus();
				break;
			}
		}
	}*/

	if (settings.contains("editor/visible")) {
		if (!settings.value("editor/visible").toBool())
			hideEditor();
	}

	settings.endGroup();

	if (firstTime)
	{
		fileBrowserDock_->hide();
		codeBrowserDock_->hide();
		toolStack_->hide();
		QPoint initialSize(900, 600);
		QPoint initialPos(QApplication::desktop()->screenGeometry().center() - initialSize / 2);
	#ifndef QT_MAC_USE_COCOA
		move(initialPos);
	#endif
		resize(initialSize.x(), initialSize.y());
		// fileBrowser_->resize(175, fileBrowser_->height());
		fileBrowser_->cwdSet(QDir::homePath());
	}

	if (tabWidget_->tabBar()->tabCount() == 0) newFile();
	tabWidget_->setFocus();

	/*static bool restoreFirstTime = true;
	if ( (activation()->timeLeft() <= 0) ||
	     (restoreFirstTime && (activation()->timeLeft() < intMax) && (!firstTime)) )
		QTimer::singleShot(0, this, SLOT(buyNow()));
	restoreFirstTime = false;*/
}

void QxMainWindow::saveState()
{
	/*qDebug() << "frameGeometry() =" << frameGeometry();
	qDebug() << "QApplication::desktop()->availableGeometry(this) =" << QApplication::desktop()->availableGeometry(this);*/
	bool wasMaximized = isMaximized();

	#ifdef Q_WS_X11
	if (isVisible() && ((windowState() & (Qt::WindowFullScreen | Qt::WindowMaximized)) != 0))
		showNormal();
	#endif

	QSettings settings;

	preferences_->save(&settings);

	settings.beginGroup("session");

	fileBrowser_->saveState(&settings);
	findText_->saveState(&settings);
	settings.setValue("editor/visible", tabWidget_->isVisible());
	settings.setValue("toolStack/visible", toolStack_->isVisible());
	settings.setValue("terminal/visible", terminalStack_->isVisible());
	settings.setValue("findText/visible", findText_->isVisible());

	// HACK, makes the splitter to store the width of the hidden widgets
	/*if (!toolStack_->isVisible()) {
		toolStack_->show();
		QCoreApplication::processEvents();
	}*/

	settings.setValue("mainWindow/toolbar/visible", toolbar_->isVisible());
	settings.setValue("mainWindow/wasMaximized", wasMaximized);
	settings.setValue("mainWindow/state", QMainWindow::saveState());
	settings.setValue("mainWindow/pos", pos());
	settings.setValue("mainWindow/size", size());
	settings.setValue("mainWindow/codeBrowserHack", true);
	settings.setValue("workspaceSplitter", workspaceSplitter_->saveState());
	{
		QStringList files;
		for (int i = 0; i < tabWidget_->tabBar()->tabCount(); ++i) {
			QString path = textEdit(i)->document()->filePath();
			if (path.length() > 0) {
				files << path;
				Edit* edit = tabWidget_->tabBar()->widget(i)->findChild<Edit*>();
				QString key = QString("editFocus_%1").arg(i);
				settings.setValue(key, edit == focusWidget());
			}
		}
		settings.setValue("tabWidget", files);
	}
	settings.setValue("tabWidgetIndexActive", tabWidget_->tabBar()->indexActive());
	settings.setValue("recentFiles", recentFiles_);

	settings.endGroup();

	activation()->saveState(&settings);
}

Edit* QxMainWindow::textEdit() const
{
	Edit* edit = 0;
	if (tabWidget_->tabBar()->tabCount() > 0)
		edit = textEdit(tabWidget_->tabBar()->indexActive());
	return edit;
}

Edit* QxMainWindow::textEdit(int i) const
{
	return tabWidget_->tabBar()->widget(i)->findChild<Edit*>();
}

void QxMainWindow::newFile()
{
	/*if (fileBrowser_->hasFocus())
		fileBrowser_->createFile();
	else*/
		openFile(QString());
}

void QxMainWindow::openFile()
{
	QStringList paths =
		QFileDialog::getOpenFileNames(
			this,
			QString() /* caption */,
			QDir::currentPath() /* dir */,
			QString() /* filter */,
			0 /* selected filter */
		);

	for (int i = 0; i < paths.count(); ++i)
		openFile(paths.at(i));
}

void QxMainWindow::openFile(QAction* action)
{
	openFile(action->data().toString());
}

void QxMainWindow::openFile(QString path, bool* failed)
{
	Ref<QxFileOpenRequest, Owner> request =
		new QxFileOpenRequest(
			((failed) ? 0 : QxFileOpenRequest::Interactive) | QxFileOpenRequest::CurrentTabWidget,
			path
		);
	openFile(request);
	if (failed)
		*failed = request->failed_;
}

void QxMainWindow::openFile(int tabIndex, QUrl url, bool* failed)
{
	Ref<QxFileOpenRequest, Owner> request =
		new QxFileOpenRequest(
			(failed) ? 0 : QxFileOpenRequest::Interactive,
			tabWidget_->tabBar(),
			tabIndex,
			url
		);
	openFile(request);
	if (failed)
		*failed = request->failed_;
}

void QxMainWindow::openFile(Ref<QxFileOpenRequest, Owner> request)
{
	QString path = (request->url_.isEmpty()) ? QString() : request->url_.toLocalFile();
	QByteArray cache;
	request->isBinary_ = false;
	request->failed_ = false;

	if ((request->mode_ & QxFileOpenRequest::CurrentTabWidget) != 0) {
		request->tabBar_ = tabWidget_->tabBar();
		request->tabIndex_ = tabWidget_->tabBar()->tabCount();
	}

	if ((request->mode_ & QxFileOpenRequest::Interactive) != 0) {
		if ((windowState() & Qt::WindowMinimized) != 0)
			setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
	}

	if (path != QString())
	{
		QFileInfo fileInfo(path);

		// prevent opening same file twice
		for (int i = 0, n = tabWidget_->tabBar()->tabCount(); i < n; ++i) {
			Edit* edit = textEdit(i);
			if (!edit->document()->filePath().isEmpty())
				if (QFileInfo(edit->document()->filePath()) == fileInfo) {
					if ((request->mode_ & QxFileOpenRequest::Interactive) != 0)
						tabWidget_->tabBar()->activate(i);
					request->document_ = edit->document();
					request->edit_ = edit;
					return;
				}
		}

		if (!request->document_)
		{
			QFile* file = new QFile(path);
			while (!file->open(QIODevice::ReadOnly)) {
				if ((!path.contains(".js")) && QFileInfo(path + ".js").exists()) {
					// workaround HACK, to open files from QtScript exceptions
					delete file;
					file = new QFile(path + ".js");
					continue;
				}
				delete file;
				request->failed_ = true;
				if ((request->mode_ & QxFileOpenRequest::Interactive) != 0) {
					QMessageBox* critical = new QMessageBox(this);
					critical->setWindowTitle(qApp->applicationName());
					critical->setIcon(QMessageBox::Critical);
					critical->setText(tr("Unable to open file \"%1\".").arg(path));
					if (!request->failureHint_.isEmpty())
						critical->setInformativeText(request->failureHint_);
					critical->setStandardButtons(QMessageBox::Ok);
					critical->exec();
					delete critical;
				}
				return;
			}
			cache = file->readAll();
			Ref<Utf8Decoder, Owner> source = new Utf8Decoder((uint8_t*)cache.data(), cache.size());
			bool valid = true;
			try {
				for (uchar_t ch; source->read(&ch););
			}
			catch (StreamIoException& ex) {
				valid = false;
			}
			request->isBinary_ = !valid;
			file->close();
			delete file;
		}
	}
	else {
		if (request->document_)
		{
			// prevent opening new document twice
			for (int i = 0, n = tabWidget_->tabBar()->tabCount(); i < n; ++i) {
				Edit* edit = textEdit(i);
				if (edit->document() == request->document_) {
					if ((request->mode_ & QxFileOpenRequest::Interactive) != 0)
						tabWidget_->tabBar()->activate(i);
					request->document_ = edit->document();
					request->edit_ = edit;
					return;
				}
			}
		}
	}

	if (request->isBinary_)
	{
		if ((request->mode_ & QxFileOpenRequest::Interactive) != 0)
		{
			QMessageBox* question = new QMessageBox(this);
			question->setWindowTitle(qApp->applicationName());
			question->setIcon(QMessageBox::Warning);
			question->setText(tr("Are you sure you want to open this file?"));
			question->setInformativeText(
				tr("The file you are trying to open does not conform to UTF8. ") +
				tr("<app> cannot handle non-UTF8 or binary files correctly.").replace("<app>", qApp->applicationName())
			);
			question->setStandardButtons(QMessageBox::Open|QMessageBox::Cancel);
			if (question->exec() == QMessageBox::Cancel) {
				delete question;
				request->failed_ = true;
				return;
			}
			delete question;
		}
		else if (!request->acceptBinary_)
		{
			request->failed_ = true;
			return;
		}
	}

	QWidget* carrier = 0;
	Edit* edit = 0;

	if (request->tabBar_) {
		carrier = new QWidget;
		carrier->setAttribute(Qt::WA_DeleteOnClose, true);
		edit = new Edit(carrier);
		carrier->setFocusProxy(edit);
		QScrollBar* vScroll = new QScrollBar(Qt::Vertical, carrier);
		edit->setVerticalScrollBar(vScroll);
		QHBoxLayout* cell0 = new QHBoxLayout;
		cell0->setSpacing(0);
		cell0->setMargin(0);
		cell0->addWidget(edit);
		cell0->addWidget(vScroll);
		carrier->setLayout(cell0);
		edit->addActions(fileWidgetActions_);
		connect(findText_, SIGNAL(showFindStatusChanged(bool)), edit, SLOT(setShowFindResult(bool)));
	}

	if (!request->document_) request->document_ = new Document;
	Ref<Document, Owner> doc = request->document_;
	if (edit) {
		edit->setDocument(doc);
		edit->moveCursor(0, 0);
		preferences_->setup(edit);

		connect(doc, SIGNAL(isModifiedChanged(bool)), this, SLOT(isModifiedChanged(bool)));
		connect(doc, SIGNAL(changed(Ref<Delta, Owner>)), findText_, SLOT(updateMatchStatus(Ref<Delta, Owner>)));
		connect(doc, SIGNAL(highlighterReady()), findText_, SLOT(updateMatchStatus()));
		edit->setContextMenu(createEditMenu(edit));
	}
	else {
		Ref<ThemeManager> themeManager = charcoalDirectory_->themeManager();
		Ref<ViewColors> colors = themeManager->activeTheme()->viewColors();

		// another HACK, document should associate view options, view should take a copy of it on setDocument()
		doc->setHistoryEnabled(true);
		doc->setDefaultStyle(new Style(colors->foregroundColor_, colors->backgroundColor_));
		doc->setTabWidth(preferences_->editorMetrics()->tabWidth_);
	}

	if (cache.size() > 0) {
		if (!cache.contains('\n')) {
			// automatically convert line endings of Apple legacy
			if (cache.contains('\r'))
				cache.replace('\r', '\n');
		}
		QTextStream source(cache);
		source.setCodec(QTextCodec::codecForName("UTF-8"));
		doc->load(&source);
		if (doc->maxLineLengthExceeded()) {
			request->failed_ = true;
			if ((request->mode_ & QxFileOpenRequest::Interactive) != 0)
				QMessageBox::critical(
					this, qApp->applicationName(),
					QString(tr("Failed to load file \"%1\", maximum line length(%2) exceeded.")).arg(path).arg(doc->maxLineLength())
				);
			return;
		}
	}
	doc->setFilePath(path);

	if (!doc->highlighter()) {
		Ref<LanguageStack> stack = languageManager_->stackByFileName(QFileInfo(path).fileName());
		if (stack)
			doc->setHighlighter(new Highlighter(stack, doc));
	}

	if (edit) {
		if (!edit->property("language").isValid()) {
			// HACK, doc should associate the 'language' property...
			Ref<LanguageStack> stack;
			if (doc->highlighter())
				stack = doc->highlighter()->languageStack();
			else
				stack = languageManager_->stackByFileName(QFileInfo(path).fileName());

			if (stack)
				edit->setProperty("language", stack->displayName());
		}

		if (path != QString()) {
			fileSystemWatcher_->addPath(path);

			if ((request->mode_ & QxFileOpenRequest::Interactive) != 0)
			{
				recentFiles_.prepend(path);
				if (recentFiles_.count() > 16)
					recentFiles_.removeLast();
				updateRecentFilesMenu();
			}

			if (cache.size() < 1000000)
				codetips::WordPredictor::instance()->extractWords(edit->context()->path(), edit->context()->text());
		}
		QString label = (path == QString()) ? tr("<new>") : QFileInfo(path).fileName();
		request->tabBar_->openTab(request->tabIndex_, label, QPixmap(), carrier);
	}

	request->edit_ = edit;
}

String QxMainWindow::openRedirect(String path)
{
	// qDebug() << "QxMainWindow::openRedirect(): path =" << path->data();
	#ifdef Q_WS_MAC
	String pathToLower = path.toLower();
	#endif
	for (int i = 0, n = tabWidget_->tabBar()->tabCount(); i < n; ++i) {
		Edit* edit = textEdit(i);
		Document* doc = edit->document();
		if (doc->isModified()) {
			String origPath = doc->filePath().toUtf8().constData();
			// qDebug() << "QxMainWindow::openRedirect(): origPath =" << origPath->data();
			#ifdef Q_WS_MAC
			if (pathToLower == origPath.toLower()) {
			#else
			if (path == origPath) {
			#endif
				Ref<File, Owner> tmp = File::temp();
				tmp->write(edit->context()->text());
				// qDebug() << "QxMainWindow::openRedirect(): writing" << origPath->data() << "to" << tmp->path()->data();
				return tmp->path();
			}
		}
	}
	return path;
}

void QxMainWindow::closeFile()
{
	if (tabWidget_->tabBar()->indexActive() != -1)
		closeFile(tabWidget_->tabBar()->indexActive());
}

void QxMainWindow::closeFile(int tabIndex)
{
	setWindowTitle(qApp->applicationName());

	Edit* edit = textEdit(tabIndex);
	Document* doc = edit->document();

	if (doc->isModified())
	{
		int choice = QMessageBox::question(this, qApp->applicationName(),
			tr("The file has been modified.\n"
			   "Do you want to save your changes?"),
			QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel,
			QMessageBox::Save
		);

		if (choice == QMessageBox::Cancel)
			return;

		if (choice == QMessageBox::Save)
			saveFile(tabIndex);
	}

	if (doc->filePath() != QString())
		fileSystemWatcher_->removePath(doc->filePath());

	tabWidget_->tabBar()->closeTab(tabWidget_->tabBar()->indexActive());
}

void QxMainWindow::closeFileAll(bool interactive)
{
	if (interactive) {
		bool cancel = false;
		checkBeforeCloseAll(&cancel);
		if (cancel) return;
	}
	while (tabWidget_->tabBar()->tabCount() > 0) {
		int i = tabWidget_->tabBar()->tabCount() - 1;
		Document* doc = textEdit(i)->document();
		if (doc->filePath() != QString())
			fileSystemWatcher_->removePath(doc->filePath());
		tabWidget_->tabBar()->closeTab(i);
	}
}

void QxMainWindow::saveFile(QString path, Ref<Document> doc)
{
	QFile file(path);
	if (file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
		QTextStream sink(&file);
		sink.setCodec(QTextCodec::codecForName("UTF-8"));
		doc->save(&sink);
		file.close();
	}
	if (file.error() != QFile::NoError) {
		QMessageBox::critical(
			this, qApp->applicationName(),
			QString("Failed to save file \"%1\".").arg(path)
		);
	}
}

void QxMainWindow::saveFile()
{
	if (tabWidget_->tabBar()->indexActive() != -1)
		saveFile(tabWidget_->tabBar()->indexActive());
}

void QxMainWindow::saveFile(int tabIndex)
{
	Edit* edit = textEdit(tabIndex);
	Document* doc = edit->document();
	QString path = doc->filePath();

	if (path == QString()) {
		saveFileAs(tabIndex);
		return;
	}

	fileSystemWatcher_->removePath(path);
	saveFile(path, doc);
	fileSystemWatcher_->addPath(path);

	if (QFileInfo(path).size() < 1000000)
		codetips::WordPredictor::instance()->extractWords(edit->context()->path(), edit->context()->text());
}

void QxMainWindow::saveFileAs()
{
	if (tabWidget_->tabBar()->indexActive() != -1)
		saveFileAs(tabWidget_->tabBar()->indexActive());
}

void QxMainWindow::saveFileAs(int tabIndex)
{
	Edit* edit = textEdit(tabIndex);
	Document* doc = edit->document();

	QString newPath = QFileDialog::getSaveFileName(
		this,
		QString() /* caption */,
		(doc->filePath() != QString()) ? doc->filePath() : QDir::currentPath(),
		QString() /* filter */,
		0 /* selected filter */
		// ,QFileDialog::DontUseSheet
	);

	if (newPath == QString()) return;

	saveFile(newPath, doc);

	if (!recentFiles_.contains(newPath)) {
		recentFiles_.prepend(newPath);
		if (recentFiles_.count() > 16)
			recentFiles_.removeLast();
		updateRecentFilesMenu();
	}

	if (doc->filePath() != QString()) {
		fileSystemWatcher_->removePath(doc->filePath());
	}

	doc->setFilePath(newPath);
	fileSystemWatcher_->addPath(newPath);
	// if (edit == QApplication::focusWidget())
	setWindowTitle(QString("%1 %2 %3").arg(newPath).arg(QChar(0x2014)).arg(qApp->applicationName()));

	QFileInfo info(newPath);
	QString fileName = info.fileName();
	tabWidget_->tabBar()->setLabel(tabIndex, fileName);
	Ref<LanguageStack> stack = languageManager_->stackByFileName(fileName);
	if (stack) {
		edit->setHighlighter(new Highlighter(stack, edit->document()));
		edit->setProperty("language", stack->displayName());
	}
}

void QxMainWindow::saveFileAll()
{
	for (int i = 0; i < tabWidget_->tabBar()->tabCount(); ++i) {
		Edit* edit = textEdit(i);
		Document* doc = edit->document();
		if (doc->isModified())
			saveFile(i);
	}
}

void QxMainWindow::quit()
{
	#ifdef Q_WS_MAC
	qApp->setQuitOnLastWindowClosed(true);
	#endif
	close();
}

void QxMainWindow::gotoHere()
{
	Edit* edit = textEdit();
	if (!edit) return;

	Document* doc = edit->document();
	QString path = doc->filePath();

	fileBrowser_->cwdSet(QFileInfo(path).dir().canonicalPath());
}

void QxMainWindow::reloadFile()
{
	Edit* edit = textEdit();
	if (!edit) return;

	Document* doc = edit->document();
	QString path = doc->filePath();

	if (path == QString())
		return;

	QFile* file = new QFile(path);
	if (!file->open(QIODevice::ReadOnly)) {
		QMessageBox::critical(
			this, qApp->applicationName(),
			QString("Unable to read file \"%1\".").arg(path)
		);
		return;
	}
	{
		QTextStream source(file);
		source.setCodec(QTextCodec::codecForName("UTF-8"));
		doc->load(&source);
		if (doc->maxLineLengthExceeded()) {
			QMessageBox::critical(
				this, qApp->applicationName(),
				QString(tr("Failed to reload file \"%1\", maximum line length(%2) exceeded.")).arg(path).arg(doc->maxLineLength())
			);
			return;
		}
		// workaround HACKs, doc should inform edit about state change on its own
		edit->updateVerticalScrollRange();
		edit->moveCursor(0, 0);
		edit->keepInView();
		edit->update();
	}
	file->close();
}

void QxMainWindow::exportToHtml()
{
	QString htmlText;
	{
		Edit* edit = textEdit();
		if (!edit) return;

		QTextStream textSink(&htmlText);
		// textSink.setCodec(QTextCodec::codecForName("UTF-8"));
		edit->exportHtml(&textSink);
	}

	{
		Ref<Document, Owner> doc = new Document;
		doc->push(0, 0, 0, 0, htmlText);
		Ref<LanguageStack> stack = languageManager_->stackByFileName("noname.html");
		if (stack)
			doc->setHighlighter(new Highlighter(stack, doc));

		Ref<QxFileOpenRequest, Owner> request = new QxFileOpenRequest(QxFileOpenRequest::CurrentTabWidget, doc);
		openFile(request);
	}
}

void QxMainWindow::printFile()
{
	Edit* edit = textEdit();
	if (!edit) return;

	QPrinter printer(QPrinter::ScreenResolution);
	QPrintDialog dialog(&printer, this);

	if (dialog.exec() == QDialog::Accepted)
		edit->print(
			&printer,
			preferences_->printingMetrics(),
			preferences_->printingPageHeader(),
			preferences_->printingPageBorder()
		);
}

void QxMainWindow::changeActiveEdit(QWidget* old, QWidget* now)
{
	if (old) {
		Edit* edit = qobject_cast<Edit*>(old);
		if (edit)
			disconnect(edit, SIGNAL(cursorMoved(int, int)), this, SLOT(updateCursorPos(int, int)));
	}
	if (now) {
		Edit* edit = now->findChild<Edit*>();
		if (edit) {
			languageButton_->visual()->setText(edit->property("language").toString());

			if (editMenuAction_->menu() != edit->contextMenu())
				editMenuAction_->setMenu(edit->contextMenu());

			QString path = edit->document()->filePath();
			if (path == QString()) path = "<new>";
			setWindowTitle(QString("%1 %2 %3").arg(path).arg(QChar(0x2014)).arg(qApp->applicationName()));
			disconnect(edit, SIGNAL(cursorMoved(int, int)), this, SLOT(updateCursorPos(int, int)));
			connect(edit, SIGNAL(cursorMoved(int, int)), this, SLOT(updateCursorPos(int, int)), Qt::QueuedConnection);
			updateCursorPos(edit->cursorLine(), edit->cursorColumn());

			findText_->setTextView(edit);

			codeBrowser_->display(edit->document(), false);
		}
		else {
			qDebug() << "QxMainWindow::changeActiveEdit(): edit == 0";
		}
	}
}

void QxMainWindow::lastTabClosed()
{
	editMenuAction_->setMenu(editMenuDummy_);
}

void QxMainWindow::gotoLineDialog()
{
	Edit* edit = textEdit();
	if (!edit) return;

	QxDialog* dialog = new QxDialog(edit);
	dialog->setWindowTitle(tr("Goto"));
	QLineEdit* input = new QLineEdit(dialog);
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(new QLabel(tr("Line"), dialog));
	layout->addWidget(input);
	dialog->setLayout(layout);

	if (dialog->exec() == QDialog::Accepted) {
		bool ok = false;
		int y = input->text().toInt(&ok);
		if (ok) {
			edit->gotoLine(y - 1);
			edit->setFocus();
		}
	}

	delete dialog;
}

void QxMainWindow::cwdDialog()
{
	QxCurrentDirDialog dialog(this);
	QString path = dialog.exec();
	if (path != QString())
		fileBrowser_->cwdSet(path);
}

void QxMainWindow::chooseLanguage()
{
	Edit* edit = textEdit();
	if (!edit) return;
	QAction* action = qobject_cast<QAction*>(sender());
	Ref<LanguageStack> stack = languageManager_->stackByIndex(action->data().toInt());
	edit->setHighlighter(new Highlighter(stack, edit->document()));
	edit->setProperty("language", stack->displayName());
	languageButton_->visual()->setText(stack->displayName());
}

void QxMainWindow::updateCommands(Ref<QxCommandsList> commands)
{
	{	// update menu

		QPointer<QMenu> oldMenu = commandsMenuAction_->menu();
		QMenu* menu = new QMenu(this);
		if (commandsSetup_)
			delete commandsSetup_;
		commandsSetup_ = menu->addAction(tr("Set&up"));
		commandsSetup_->setShortcut(tr("Ctrl+Alt+,"));
		connect(commandsSetup_, SIGNAL(triggered()), preferences_, SLOT(showTabCommands()));
		menu->addSeparator();
		for (int i = 0; i < commands->length(); ++i)
			menu->addAction(commands->get(i)->action());
		commandsMenuAction_->setMenu(menu);
		if (oldMenu)
			delete oldMenu;
	}

	{	// update toolbar

		for (int i = 0; i < toolbarCommands_.length(); ++i)
			toolbar_->removeAction(toolbarCommands_.get(i));

		toolbarCommands_.clear();

		QAction* before = firstCommand_;
		for (int i = 0; i < commands->length(); ++i) {
			QxCommand* cmd = commands->get(i);
			if (!cmd->icon_.isNull()) {
				toolbar_->insertAction(before, cmd->action());
				toolbarCommands_.append(cmd->action());
			}
		}
	}
}

void QxMainWindow::updateViewMenu()
{
	toolbarShow_->setChecked(toolbar_->isVisible());
	windowMaximized_->setChecked(isMaximized());
}

void QxMainWindow::updateLanguageMenu()
{
	if (languageMenu_)
		delete languageMenu_;

	languageMenu_ = new QMenu(this);

	QMap<QString, int> map;
 	for (int i = 0, n = languageManager_->numStacks() - 1; i < n; ++i) {
		Ref<LanguageStack> stack = languageManager_->stackByIndex(i);
		map.insertMulti(stack->displayName().toLower(), i);
	}

	QMapIterator<QString, int> mi(map);
	while (mi.hasNext()) {
		mi.next();
		Ref<LanguageStack> stack = languageManager_->stackByIndex(mi.value());
		QAction* action = languageMenu_->addAction(stack->displayName(), this, SLOT(chooseLanguage()));
		action->setData(mi.value());
	}

	languageMenu_->addSeparator();

	int lastIndex = languageManager_->numStacks() - 1;
	Ref<LanguageStack> stack = languageManager_->stackByIndex(lastIndex);
	QAction* action = languageMenu_->addAction(stack->displayName(), this, SLOT(chooseLanguage()));
	action->setData(lastIndex);

	languageButton_->setMenu(languageMenu_);
}

void QxMainWindow::updateRecentFilesMenu()
{
	QMenu* menu = fileOpenRecent_->menu();
	if (menu) delete menu;
	menu = new QMenu(this);
	QFileIconProvider iconProvider;

	for (int i = 0, n = recentFiles_.count(); i < n; ++i)
		menu->addAction(iconProvider.icon(QFileInfo(recentFiles_.at(i))), recentFiles_.at(i))->setData(recentFiles_.at(i));
	connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(openFile(QAction*)));
	fileOpenRecent_->setMenu(menu);
}

void QxMainWindow::updateCursorPos(int line, int column)
{
	gotoLineButton_->visual()->setText(QString().sprintf("%d: %d", line + 1, column + 1));
}

void QxMainWindow::isModifiedChanged(bool value)
{
	for (int i = 0; i < tabWidget_->tabBar()->tabCount(); ++i)
	{
		Edit* edit = textEdit(i);
		if (!edit) continue;
		Document* doc = edit->document();
		if (doc == sender()) {
			QString label = QFileInfo(doc->filePath()).fileName();
			if (label == QString())
				label = tr("<new>");
			if (doc->isModified())
				label += " *";
			tabWidget_->tabBar()->setLabel(i, label);
		}
	}
}

void QxMainWindow::updateCodeBrowser()
{
	Edit* edit = textEdit();
	if (edit)
		codeBrowser_->display(edit->document());
}

void QxMainWindow::toggleFullscreenMode()
{
	if ((windowState() & Qt::WindowFullScreen) != 0) {
		/*#ifdef Q_WS_MAC
			// work around issues with toolbar layout, HACK
			#ifdef QT_MAC_USE_COCOA
				toolbar_->setVisible(false);
				showNormal();
				toolbar_->setVisible(toolbarVisibleSaved_);
			#else
				// hide();
				showNormal();
				activateWindow();
				addToolBar(toolbar_);
				toolbar_->setVisible(toolbarVisibleSaved_);
				move(windowPosSaved_);
			#endif
		#else*/
		showNormal();
		// #endif
	}
	else {
		/*#ifdef Q_WS_MAC
			toolbarVisibleSaved_ = toolbar_->isVisible();
			#ifndef QT_MAC_USE_COCOA
				windowPosSaved_ = pos();
				hide();
				removeToolBar(toolbar_);
			#endif
		#endif*/
		showFullScreen();
	}
}

void QxMainWindow::toggleShowMaximized()
{
	if ((windowState() & (Qt::WindowFullScreen | Qt::WindowMaximized)) != 0)
		showNormal();
	else
		showMaximized();
}

void QxMainWindow::toggleShowToolbar()
{
	#ifdef QT_MAC_USE_COCOA
	bool kickMe = windowIsFitted();
	toolbar_->setVisible(!toolbar_->isVisible());
	if (kickMe) {
		resize(width()+1, height());
		resize(width()-1, height());
	}
	#else
	toolbar_->setVisible(!toolbar_->isVisible());
	#endif
}

void QxMainWindow::showEditor()
{
	if (tabWidget_->isVisible()) {
		if (!tabWidget_->inFocus())
			tabWidget_->setFocus();
	}
	else {
		tabWidget_->show();
		tabWidget_->setFocus();
		findText_->setScope(QxFindText::CurrentFile);
	}
}

void QxMainWindow::hideEditor()
{
	tabWidget_->hide();
	findText_->setScope(QxFindText::Terminal);
}

void QxMainWindow::showTerminal()
{
	if (terminalStack_->isVisible()) {
		if (!terminalStack_->inFocus())
			terminalStack_->setFocus();
	}
	else {
		terminalSwitch_->press();
	}
}

void QxMainWindow::hideTerminal()
{
	if (terminalStack_->isVisible())
		terminalSwitch_->press();
}

void QxMainWindow::toggleTerminal()
{
	terminalSwitch_->press();
}

void QxMainWindow::showFindText()
{
	if (findText_->isVisible()) {
		if (!findText_->hasFocus())
			findText_->setFocus();
	}
	else {
		findTextSwitch_->press();
	}
	findText_->selectPatternText();
}

void QxMainWindow::hideFindText()
{
	if (findText_->isVisible())
		findTextSwitch_->press();
}

void QxMainWindow::showFileBrowser()
{
	if (fileBrowserDock_->isVisible()) {
		if (!fileBrowser_->hasFocus())
			fileBrowser_->setFocus();
	}
	else {
		fileBrowserDock_->show();
		fileBrowser_->setFocus();
	}
}

void QxMainWindow::hideFileBrowser()
{
	if (fileBrowserDock_->isVisible())
		toggleFileBrowser();
}

void QxMainWindow::toggleFileBrowser()
{
	bool visible = fileBrowserDock_->isVisible();
	fileBrowserDock_->setVisible(!visible);
	if (!visible)
		fileBrowser_->setFocus();
	else if (tabWidget_->isVisible())
		tabWidget_->setFocus();
	else if (terminalStack_->isVisible())
		terminalStack_->setFocus();
}

void QxMainWindow::showCodeBrowser()
{
	if (codeBrowserDock_->isVisible()) {
		if (!codeBrowser_->hasFocus())
			codeBrowser_->setFocus();
	}
	else {
		codeBrowserDock_->show();
		codeBrowser_->setFocus();
	}
}

void QxMainWindow::hideCodeBrowser()
{
	if (codeBrowserDock_->isVisible())
		toggleCodeBrowser();
}

void QxMainWindow::toggleCodeBrowser()
{
	bool visible = codeBrowserDock_->isVisible();
	codeBrowserDock_->setVisible(!visible);
	if (!visible)
		codeBrowser_->setFocus();
	else if (tabWidget_->isVisible())
		tabWidget_->setFocus();
	else if (terminalStack_->isVisible())
		terminalStack_->setFocus();
}

void QxMainWindow::editorShowMaximized()
{
	if (terminalSwitch_->isPressed() || findTextSwitch_->isPressed()) {
		if (!tabWidget_->isVisible()) tabWidget_->show();
		terminalSwitch_->toggle(false);
		findTextSwitch_->toggle(false);
	}
	else {
		terminalSwitch_->toggle(true); // mild LOGIC HACK
	}
}

void QxMainWindow::terminalShowMaximized()
{
	if (tabWidget_->isVisible()) {
		tabWidget_->hide();
		if (!toolStack_->isVisible()) toolStack_->show();
		terminalSwitch_->toggle(true);
	}
	else {
		tabWidget_->show();
	}
}

void QxMainWindow::openTab()
{
	if (tabWidget_->inFocus())
		newFile();
	else if (terminalStack_->inFocus())
		terminalStack_->openTerminal();
}

void QxMainWindow::closeTab()
{
	if (terminalStack_->inFocus())
		terminalStack_->closeTerminal();
	else /*if (tabWidget_->inFocus())*/
		closeFile();
}

void QxMainWindow::previousTab()
{
	if (tabWidget_->inFocus()  || (terminalStack_->tabBar()->tabCount() <= 1)) {
		if (tabWidget_->tabBar()->indexActive() > 0)
			tabWidget_->tabBar()->activate(tabWidget_->tabBar()->indexActive() - 1);
	}
	else if (terminalStack_->inFocus()) {
		if (terminalStack_->tabBar()->indexActive() > 0)
			terminalStack_->tabBar()->activate(terminalStack_->tabBar()->indexActive() - 1);
	}
}

void QxMainWindow::nextTab()
{
	if (tabWidget_->inFocus() || (terminalStack_->tabBar()->tabCount() <= 1)) {
		if (tabWidget_->tabBar()->indexActive() < tabWidget_->tabBar()->tabCount() - 1)
			tabWidget_->tabBar()->activate(tabWidget_->tabBar()->indexActive() + 1);
	}
	else if (terminalStack_->inFocus()) {
		if (terminalStack_->tabBar()->indexActive() < terminalStack_->tabBar()->tabCount() - 1)
			terminalStack_->tabBar()->activate(terminalStack_->tabBar()->indexActive() + 1);
	}
}

void QxMainWindow::dragTabLeft()
{
	// qDebug() << "QxMainWindow::dragTabLeft()";
	if (tabWidget_->inFocus()  || (terminalStack_->tabBar()->tabCount() <= 1))
		tabWidget_->tabBar()->dragTabLeft();
	else if (terminalStack_->inFocus())
		terminalStack_->tabBar()->dragTabLeft();
	tabDragging_ = true;
}

void QxMainWindow::dragTabRight()
{
	// qDebug() << "QxMainWindow::dragTabRight()";
	if (tabWidget_->inFocus()  || (terminalStack_->tabBar()->tabCount() <= 1))
		tabWidget_->tabBar()->dragTabRight();
	else if (terminalStack_->inFocus())
		terminalStack_->tabBar()->dragTabRight();
	tabDragging_ = true;
}

void QxMainWindow::dropTab()
{
	if (!tabDragging_) return;
	// qDebug() << "QxMainWindow::dropTab()";
	if (tabWidget_->inFocus()  || (terminalStack_->tabBar()->tabCount() <= 1))
		tabWidget_->tabBar()->dropTab();
	else if (terminalStack_->inFocus())
		terminalStack_->tabBar()->dropTab();
	tabDragging_ = false;
}

void QxMainWindow::openLink()
{
	if (terminalStack_->isVisible()) {
		QxVideoTerminal* vt = terminalStack_->activeTerminal();
		if (vt) vt->openLink();
	}
	else if (findText_->isVisible()) {
		findText_->openLink();
	}
}

void QxMainWindow::firstLink()
{
	if (terminalStack_->isVisible()) {
		QxVideoTerminal* vt = terminalStack_->activeTerminal();
		if (vt) vt->firstLink();
	}
	else if (findText_->isVisible()) {
		findText_->firstLink();
	}
}

void QxMainWindow::lastLink()
{
	if (terminalStack_->isVisible()) {
		QxVideoTerminal* vt = terminalStack_->activeTerminal();
		if (vt) vt->lastLink();
	}
	else if (findText_->isVisible()) {
		findText_->lastLink();
	}
}

void QxMainWindow::previousLink()
{
	if (terminalStack_->isVisible()) {
		QxVideoTerminal* vt = terminalStack_->activeTerminal();
		if (vt) vt->previousLink();
	}
	else if (findText_->isVisible()) {
		findText_->previousLink();
	}
}

void QxMainWindow::nextLink()
{
	if (terminalStack_->isVisible()) {
		QxVideoTerminal* vt = terminalStack_->activeTerminal();
		if (vt) vt->nextLink();
	}
	else if (findText_->isVisible()) {
		findText_->nextLink();
	}
}

QString QxMainWindow::lookupPath(QString path, QDir dir)
{
	if (QFileInfo(path).isAbsolute()) {
		if (QFileInfo(path).exists())
			return path;
		else
			path = path.mid(1, path.length() - 1);
	}
	if (!dir.exists(path)) {
		if (!dir.isRoot())
			if (dir.cdUp())
				path = lookupPath(path, dir);
	}
	else {
		path = dir.filePath(path);
	}
	return path;
}

void QxMainWindow::gotoLine(int y)
{
	Edit* edit = textEdit();
	if (!edit) return;
	edit->moveCursor(y, 0);
	edit->keepInView();
	edit->setFocus();
}

void QxMainWindow::gotoLink(Ref<Link, Owner> link)
{
	if (link->type_ == Link::FilePos) {
		QString path = lookupPath(link->targetPath_, QDir::current());

		{
			QList<QUrl> urls = fileBrowser_->bookmarkedDirs();
			QString cwd = QDir::currentPath();
			if (!urls.contains(cwd)) urls.append(cwd);
			for (int i = 0, n = urls.count(); (i < n) && (!QFileInfo(path).exists()); ++i)
				path = lookupPath(link->targetPath_, urls.at(i).path());
		}

		Ref<QxFileOpenRequest, Owner> request =
			new QxFileOpenRequest(
				QxFileOpenRequest::Interactive|
				QxFileOpenRequest::CurrentTabWidget,
				path
			);

		request->failureHint_ = tr(
			"You may need to add the directory the file is located in to the bookmarked directories. "
			"See the bookmark icon in the file browser."
		);

		openFile(request);

		if (!request->failed_) {
			int cy = link->targetRow_ - 1;
			int cx = link->targetColumn_;
			if (cy >= 0) {
				View* view = request->edit_;
				view->highlighterYield();
				int cy2 = cy;
				int cx2 = 0;
				while (cx > 0) {
					view->document()->stepForward(&cy2, &cx2);
					if (cy2 != cy) break;
					--cx;
				}
				view->moveCursor(cy, cx2);
				view->keepInView();
				view->blinkRestart();
				view->update();
				tabWidget_->setFocus();
				view->highlighterResume();
			}
		}
	}
	else if (link->type_ == Link::Uri) {
		QDesktopServices::openUrl(link->targetPath_);
	}
}

void QxMainWindow::gotoMatch(Ref<Document> document, int my, int mx0, int mx1)
{
	View* view = 0;

	if (terminalStack_->activate(document)) {
		switchTerminal(true);
		view = terminalStack_->activeTerminal();
	}
	else {
		Ref<QxFileOpenRequest, Owner> request =
			new QxFileOpenRequest(
				QxFileOpenRequest::Interactive|
				QxFileOpenRequest::CurrentTabWidget,
				document
			);
		openFile(request);
		view = request->edit_;
		if (my != -1) {
			view->highlighterYield();
			view->moveCursor(my, mx1);
			view->highlighterResume();
		}
	}

	if (view) {
		view->highlighterYield();
		if (my != -1) {
			if (mx0 != mx1)
				view->select(my, mx0, my, mx1);
			view->keepInView(my, mx1);
		}
		view->setFocus();
		view->update();
		view->highlighterResume();
	}

	statusBar_->display(QxStatusBar::Center, "Press <ESC> to toggle off/on match highlights");
}

void QxMainWindow::matchStatusMessage(QString message)
{
	statusBar_->display(QxStatusBar::Center, message);
}

void QxMainWindow::switchTerminal(bool on)
{
	if (on) {
		if (toolStackLayout_->currentWidget() == findText_)
			findTextSwitch_->toggle(false);
		toolStackLayout_->setCurrentWidget(terminalStack_);
		terminalStack_->setFocus();
	}
	else {
		QWidget* w = textEdit();
		if (w) w->setFocus();
	}
	toolStack_->setVisible(on);
}

void QxMainWindow::switchFindText(bool on)
{
	if (on) {
		if (toolStackLayout_->currentWidget() == terminalStack_)
			terminalSwitch_->toggle(false);
		toolStackLayout_->setCurrentWidget(findText_);
		findText_->selectPatternText();
	}
	else {
		QWidget* w = textEdit();
		if (w) w->setFocus();
	}
	toolStack_->setVisible(on);
}

void QxMainWindow::fileRenamed(QString oldPath, QString newPath)
{
	for (int i = 0; i < tabWidget_->tabBar()->tabCount(); ++i) {
		Document* doc = textEdit(i)->document();
		if (doc->filePath() == oldPath) {
			QString label = tabWidget_->tabBar()->label(i);
			label.replace(QFileInfo(oldPath).fileName(), QFileInfo(newPath).fileName());
			tabWidget_->tabBar()->setLabel(i, label);
			doc->setFilePath(newPath);
			fileSystemWatcher_->removePath(oldPath);
			fileSystemWatcher_->addPath(newPath);
			break;
		}
	}
}

void QxMainWindow::fileChanged(const QString& path)
{
	for (int i = 0; i < tabWidget_->tabBar()->tabCount(); ++i) {
		Document* doc = textEdit(i)->document();
		if (doc->filePath() == path)
			doc->markDirty();
	}
}

int QxMainWindow::fileOffset(String path, int line, int column)
{
	if ((line == 0) && (column == 0)) return 0;
	Ref<File, Owner> file = new File(path);
	file->open(File::Read);
	int y = 0, x = 0;
	off_t offs = 0;
	Ref<Utf8Decoder, Owner> source = new Utf8Decoder(file);
	for (uchar_t ch; source->read(&ch);) {
		++offs;
		++x;
		if (ch == '\n') {
			if ((y == line) && (x == column))
				break;
			++y;
			x = 0;
		}
		if ((y == line) && (x == column))
			break;
	}
	return source->byteDecoder()->numBytesRead();
}

void QxMainWindow::commandTriggered(QxCommand* cmd)
{
	int target = cmd->target_;
	QString script = cmd->script_;
	{
		Edit* edit = textEdit();
		QString filePath = (edit) ? edit->document()->filePath() : QString();
		if (filePath == QString())
			filePath = "noname.txt";
		QString dirPath = QFileInfo(filePath).dir().absolutePath();
		QString dirName = QFileInfo(dirPath).fileName();
		QString fileName = QFileInfo(filePath).fileName();
		QString fileExt = QFileInfo(filePath).completeSuffix();
		QString fileId = fileName.indexOf('.') ? fileName.left(fileName.indexOf('.')) : fileName;

		QString text, word;
		int line = 1, column = 1;

		View* view = qobject_cast<View*>(QApplication::focusWidget());
		if (view) {
			if (view->hasSelection())
				text = view->copySelection();
			word = view->wordUnderCursor();
			line = view->cursorLine();
			column = view->cursorColumn();
			column = view->document()->copySpan(line, 0, column).length();
		}

		if ((word == QString()) && (text != QString())) {
			bool isWord = true;
			for (int i = 0; (i < text.length()) && isWord; ++i)
				isWord = isWord && view->isWord(text.at(i));
			if (isWord)
				word = text;
		}

		script.replace("$DIRPATH", dirPath);
		script.replace("$DIRNAME", dirName);
		script.replace("$FILEPATH", filePath);
		script.replace("$FILENAME", fileName);
		script.replace("$FILEID", fileId);
		script.replace("$FILEEXT", fileExt);
		script.replace("$TEXT", text);
		script.replace("$WORD", word);
		script.replace("$CURSOR_LINE", QString("%1").arg(line));
		script.replace("$CURSOR_COLUMN", QString("%1").arg(column));

		if (cmd->autoSaveFile_) {
			saveFile();
			if (script.contains("$OFFSET")) {
				int offset = fileOffset(String(filePath.toUtf8().constData()), line, column);
				script.replace("$OFFSET", QString("%1").arg(offset));
			}
		}
	}

	if (script.length() > 0) {
		if ((target == QxCommand::ActiveTerminal) || (target == QxCommand::NewTerminal)) {
			if (target == QxCommand::NewTerminal) terminalStack_->openTerminal();
			QxVideoTerminal* vt = terminalStack_->activeTerminal();
			if (vt) {
				if (!terminalStack_->isVisible())
					toggleTerminal();
				vt->setAutoOpenNextLink(cmd->autoOpenNextLink_);
				QByteArray ba = script.toUtf8();
				vt->shell()->rawInput()->write(ba.data(), ba.size());
				if (script.at(script.length()-1) != '\n')
					vt->shell()->rawInput()->write("\n", 1);
			}
		}
		/*else if (target == QxCommand::TextEditor) {
			Edit* edit = textEdit();
			if (edit) edit->paste(script);
		}*/
		else if (target == QxCommand::WebBrowser) {
			if (!script.contains("//")) {
				if (!script.contains("/")) {
					script.prepend("http://");
					script.append("/");
				}
				else {
					if (script.at(0) != '/')
						script.prepend('/');
					script.prepend("file://");
				}
			}
			// qDebug() << "QDesktopServices::openUrl(" << script << ")";
			QDesktopServices::openUrl(script);
		}
	}
}

void QxMainWindow::openActivation()
{
	QxActivationDialog* dialog = new QxActivationDialog(this);
	dialog->exec();
	delete dialog;
	{
		QSettings settings;
		preferences_->save(&settings);
	}
}

void QxMainWindow::about()
{
	QxAboutDialog* dialog = new QxAboutDialog(this);
	dialog->exec();
	delete dialog;
}

void QxMainWindow::license()
{
	QxLicenseDialog* dialog = new QxLicenseDialog(this);
	dialog->exec();
	delete dialog;
}

void QxMainWindow::onlineManual()
{
	#ifdef Q_WS_MAC
	QString path = QString("file://") + QCoreApplication::applicationDirPath() + "/../Resources/manual/manual.html";
	qDebug() << "path = " << path;
	QDesktopServices::openUrl(QUrl(path));
	#else
	QDesktopServices::openUrl(QUrl(QCoreApplication::applicationDirPath() + "/../shared/manual/manual.html"));
	#endif
}

/*
void QxMainWindow::screenshot()
{
	QSvgGenerator generator;
	generator.setFileName("/home/frank/Desktop/test.svg");
	generator.setSize(QSize(width(), height()));
	generator.setViewBox(QRect(0, 0, width(), height()));
	generator.setTitle(tr("Screenshot of Pacu"));
	generator.setDescription(tr("An SVG drawing created by the QSvgGenerator."));
	QPainter p;
	p.begin(&generator);
	render(&p);
	p.end();
}
*/

void QxMainWindow::checkBeforeCloseAll(bool* cancel)
{
	QStringList modifiedFiles;
	for (int i = 0, n = tabWidget_->tabBar()->tabCount(); i < n; ++i) 	{
		Document*  doc = textEdit(i)->document();
		if (doc->isModified())
			modifiedFiles << doc->filePath();
	}

	*cancel = false;

	if (modifiedFiles.count() == 0) return;

	if (isMinimized()) showNormal();

	QMessageBox* question = new QMessageBox(this);
	question->setWindowTitle(qApp->applicationName());
	question->setIcon(QMessageBox::Question);
	question->setText(tr("Are you sure you want to close all files?"));
	question->setInformativeText(tr("Some files have been modified. Your changes will be lost."));
	question->setDetailedText(modifiedFiles.join("\n"));
	question->setStandardButtons(QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
	question->setDefaultButton(QMessageBox::Save);

	#ifdef Q_WS_MAC
	QList<QFrame*> frames = question->findChildren<QFrame*>();
	for (int i = 0; i < frames.count(); ++i) {
		QFrame* frame = frames.at(i);
		if (frame->frameShape() == QFrame::HLine)
			frame->setFrameShape(QFrame::NoFrame);
	}
	#endif

	int choice = question->exec();
	if (choice == QMessageBox::Cancel) *cancel = true;
	else if (choice == QMessageBox::Save) saveFileAll();

	delete question;
}

void QxMainWindow::closeEvent(QCloseEvent* event)
{
	bool cancel = false;
	checkBeforeCloseAll(&cancel);
	if (cancel) {
		event->ignore();
	}
	else {
		saveState();
		event->accept();
		hide();
		closeFileAll(false);
	}
}

void QxMainWindow::keyReleaseEvent(QKeyEvent* event)
{
	if ((event->key() == Qt::Key_Meta) || (event->key() == Qt::Key_Control) || (event->key() == Qt::Key_Alt))
		dropTab();
	// qDebug() << "QxMainWindow::keyReleaseEvent()";
}

void QxMainWindow::commitData(QSessionManager& manager)
{
	if (manager.allowsInteraction()) {
		bool cancel = false;
		checkBeforeCloseAll(&cancel);
		if (cancel) {
			manager.cancel();
		}
		else {
			saveState();
			hide();
			closeFileAll(false);
			manager.release();
		}
	}
}

#ifdef Q_WS_MAC
bool QxMainWindow::isMaximized() const
{
	bool max = QWidget::isMaximized();
	#ifdef Q_WS_MAC
	{
		QSize currSize = frameGeometry().size();
		QSize availSize = QApplication::desktop()->availableGeometry(this).size();
		if (currSize.width() > availSize.width()) currSize.setWidth(availSize.width());
		if (currSize.height() > availSize.height()) currSize.setHeight(availSize.height());
		max = (currSize == availSize);
	}
	#endif
	return max;
}

bool QxMainWindow::windowIsFitted() const
{
	QRect avail = QApplication::desktop()->availableGeometry(this);
	QRect frame = frameGeometry();
	QRect displ = QApplication::desktop()->screenGeometry(this);
	/*qDebug() << "avail =" << avail;
	qDebug() << "frame =" << frame;
	qDebug() << "displ =" << displ;*/
	bool fitted = avail.height() <= frame.height() /*) && (avail.bottom() == displ.bottom())*/;
	//qDebug() << "fitted =" << fitted;
	return fitted;
}

bool QxMainWindow::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::ApplicationActivate) {
		bool wasHidden = !isVisible();
		if (!firstTimeActive_) {
			if (!isVisible()) {
				restoreState();
				show();
			}
			else if (isMinimized()) {
				showNormal();
			}
		}
		#ifdef QT_MAC_USE_COCOA
		if (wasHidden || firstTimeActive_) {
			firstTimeActive_ = false;
			if (tabWidget_->isVisible())
				tabWidget_->setFocus();
			else if (terminalStack_->isVisible())
				terminalStack_->setFocus();
		}
		#endif
	}
	return event->type() == QEvent::ToolTip;
}
#endif

} // namespace pacu
