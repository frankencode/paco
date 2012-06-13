#ifndef PACU_QXMAINWINDOW_HPP
#define PACU_QXMAINWINDOW_HPP

#include <QtGui>
#include <ftl/String.hpp>
#include "UseFtl.hpp"
#include "UseVide.hpp"
#include "UsePte.hpp"
#include "QxPreferences.hpp"
#include "QxFileOpenRequest.hpp"

namespace ftl {
	class Semaphore;
}

class QDockWidget;
class QSessionManager;

namespace pte
{

class Edit;
class Highlighter;
class LanguageManager;
class CharcoalDirectory;

} // namespace pte

namespace pacu
{

class QxFileSystemWatcher;
class QxFileSearchEdit;
class QxFileBrowser;
class QxCodeBrowser;
class QxVideoTerminal;
class QxTerminalStack;
class QxStatusBar;
class QxFindText;
class QxSplitter;
class QxTabWidget;
class InterpositionProxy;

class QxMainWindow: public QMainWindow
{
	Q_OBJECT
	
public:
	QxMainWindow();
	~QxMainWindow();
	
#ifndef QT_MAC_USE_COCOA
	void reopen();
#endif
	
	String openRedirect(String path);
	
signals:
	void openFileQuick();
	
private slots:
	void buyNow();
	
	void newFile();
	void openFile();
	void openFile(QAction* action);
	void openFile(QString path, bool* failed = 0);
	void openFile(int tabIndex, QUrl url, bool* failed = 0);
	void openFile(Ref<QxFileOpenRequest, Owner> request);
	void closeFile();
	void closeFile(int tabIndex);
	void closeFileAll(bool interactive = true);
	void saveFile(QString path, Ref<Document> doc);
	void saveFile();
	void saveFile(int tabIndex);
	void saveFileAs();
	void saveFileAs(int tabIndex);
	void saveFileAll();
	void quit();
	void gotoHere();
	void reloadFile();
	void exportToHtml();
	void printFile();
	
	void changeActiveEdit(QWidget* old, QWidget* now);
	void lastTabClosed();
	
	void gotoLineDialog();
	void cwdDialog();
	
	void chooseLanguage();
	
	void updateViewMenu();
	void updateLanguageMenu();
	void updateRecentFilesMenu();
	void updateCommands(Ref<QxCommandsList> commands);
	
	void updateCursorPos(int line, int column);
	void isModifiedChanged(bool value);
	
	void updateCodeBrowser();
	
	void switchTerminal(bool on);
	void switchFindText(bool on);
	
	void toggleFullscreenMode();
	void toggleShowMaximized();
	void toggleShowToolbar();
	void showEditor();
	void hideEditor();
	void showTerminal();
	void hideTerminal();
	void toggleTerminal();
	void showFindText();
	void hideFindText();
	void showFileBrowser();
	void hideFileBrowser();
	void toggleFileBrowser();
	void showCodeBrowser();
	void hideCodeBrowser();
	void toggleCodeBrowser();
	
	void editorShowMaximized();
	void terminalShowMaximized();
	
	void openTab();
	void closeTab();
	void previousTab();
	void nextTab();
	void dragTabLeft();
	void dragTabRight();
	void dropTab();
	
	void openLink();
	void firstLink();
	void lastLink();
	void previousLink();
	void nextLink();
	
	void gotoLine(int y);
	void gotoLink(Ref<Link, Owner> link);
	void gotoMatch(Ref<Document> document, int my, int mx0, int mx1);
	void matchStatusMessage(QString message);
	
	void fileRenamed(QString oldPath, QString newPath);
	void fileChanged(const QString& path);
	
	void commandTriggered(QxCommand* cmd);
	
	void openActivation();
	void about();
	void license();
	void onlineManual();
	// void screenshot();
	
	void commitData(QSessionManager& manager);
	
private:
	static int fileOffset(String path, int line, int column);
	
	void createFileMenu();
	QMenu* createEditMenu(Edit* edit = 0);
	void createViewMenu();
	void createNavMenu();
	void createCommandsMenu();
	void createHelpMenu();
	void setupWindowIcon();
	void setupToolbar();
	void setupWorkspace();
	
	void restoreState();
	void saveState();
	
	QString lookupPath(QString path, QDir dir);
	
	Edit* textEdit() const;
	Edit* textEdit(int i) const;
	
	void checkBeforeCloseAll(bool* cancel);
	virtual void closeEvent(QCloseEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);

	
#ifdef Q_WS_MAC
	bool isMaximized() const;
	bool windowIsFitted() const;
	virtual bool eventFilter(QObject* watched, QEvent* event);
#endif
	
	QxTerminalStack* terminalStack_;
	
	QList<QAction*> fileWidgetActions_;
	QAction* fileOpenRecent_;
	QAction* fileReload_;
	QAction* fileClose_;
	
	QAction* editMenuAction_;
	QMenu* editMenuDummy_;
	
	QAction* windowMaximized_;
	QAction* toolbarShow_;
	
	QAction* commandsMenuAction_;
	QPointer<QAction> commandsSetup_;
	
	QMenu* helpMenu_;
	QPointer<QMenu> languageMenu_;
	
	QToolBar* toolbar_;
	CustomList<QAction*> toolbarCommands_;
	// QAction* mainFileBrowser_;
	// QAction* mainTerminal_;
	// QAction* mainPreferences_;
	QAction* firstCommand_;
	// QAction* mainDelete_;
	// QAction* mainFindText_;
	// QAction* mainOpacity_;
	
	QxSplitter* workspaceSplitter_;
	
	QWidget* toolStack_;
	QStackedLayout* toolStackLayout_;
	QxControl* terminalSwitch_;
	QxControl* findTextSwitch_;
	QxControl* gotoLineButton_;
	QxControl* languageButton_;
	
	QxFileSearchEdit* fileSearchEdit_;
	QDockWidget* fileBrowserDock_;
	QxFileBrowser* fileBrowser_;
	QDockWidget* codeBrowserDock_;
	QxCodeBrowser* codeBrowser_;
	QxTabWidget* tabWidget_;
	QxFindText* findText_;
	QxStatusBar* statusBar_;
	
	QxPreferences* preferences_;
	
	QPointer<QxFileSystemWatcher> fileSystemWatcher_;
	
	QStringList recentFiles_;
	#ifdef Q_WS_MAC
	bool toolbarVisibleSaved_;
	QPoint windowPosSaved_;
	#endif
	
	Ref<CharcoalDirectory, Owner> charcoalDirectory_;
	Ref<LanguageManager, Owner> languageManager_;
	
	bool appLaunchTime_;
	bool firstTimeActive_;
	bool wasFitted_;
	
	bool tabDragging_;
	
	Ref<InterpositionProxy, Owner> interpositionProxy_;
};

} // namespace pacu

#endif // PACU_QXMAINWINDOW_HPP
