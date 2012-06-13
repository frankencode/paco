#ifndef PACU_QXFILEBROWSER_HPP
#define PACU_QXFILEBROWSER_HPP

#include <QLayout>
#include "UseVide.hpp"
#include "QxFileSystemView.hpp"
#include "QxStatusBar.hpp"
#include "QxDialog.hpp"
#include "QxSplitter.hpp"
#include "QxUrlListModel.hpp"
#include "QxUrlListView.hpp"

class QFileSystemModel;
class QStackedLayout;
class QSettings;

namespace pacu
{

class QxFileBrowser: public QWidget
{
	Q_OBJECT
	
public:
	QxFileBrowser(QWidget* parent);
	
	void saveState(QSettings* settings);
	void restoreState(const QSettings* settings);
	
	QList<QUrl> bookmarkedDirs() const;
	
signals:
	void openFile(QString filePath);
	void cwdChanged(QString path);
	void fileRenamed(QString oldFilePath, QString newFilePath);
	void dirRenamed(QString oldDirPath, QString newDirPath);
	void escape();
	
public slots:
	void showHidden(bool on);
	void showDetails(bool on);
	void cwdSet(const QString& newPath);
	void createFile();
	
private slots:
	void openDefault();
	void openFile();
	void cwdSet(QAction* action);
	void cwdSet(const QModelIndex& index);
	void cdUp();
	void newNameChanged(QString s);
	void createFilePolling();
	void createDir();
	void rename();
	void delete_();
	void fileRenamed(const QString& path, const QString& oldName, const QString& newName);
	void recentShowOrHide();
	void bookmarksShowOrHide();
	void gotoRecent(const QModelIndex& index);
	void gotoRecent();
	void recentAddToBookmarks();
	void gotoBookmark(const QModelIndex& index);
	void gotoBookmark();
	void addDirBookmark();
	void cwdAddToBookmark();
	void bookmark();
	void removeBookmark();
	void bookmarksShowFullPath(bool on);
	void blockFloating(bool on);
	void adaptToDockLocation(Qt::DockWidgetArea area);

private:
	void updateStatusDisplay();
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void contextMenuEvent(QContextMenuEvent* event);
	QMenu* newDirSelector();
	void updateGotoMenu();
	void addBookmark(QUrl url);
	
	// virtual void focusInEvent(QFocusEvent* event);
	
	QxControl* gotoButton_;
	QPointer<QMenu> gotoMenu_;
	QPointer<QMenu> recentMenu_;
	QMap<QAction*, QString> actionToPath_;
	QxStatusBar* statusBar_;
	bool showHidden_;
	bool showDetails_;
	QFileSystemModel* cwdModel_;
	QxFileSystemView* dirView_;
	
	QString newFilePath_;
	
	QMenu* contextMenu_;
	QAction* openDefaultAction_;
	QAction* openFileAction_;
	QAction* createFileAction_;
	QAction* createDirAction_;
	QAction* renameAction_;
	QAction* deleteAction_;
	QAction* showHiddenAction_;
	QAction* showDetailsAction_;
	QAction* bookmarkAction_;
	
	QString newName_;
	QxDialog* createFileDialog_;
	QxDialog* createDirDialog_;
	QxDialog* renameDialog_;
	
	QStringList cwdPast_;
	QStringList cwdFuture_;
	
	QxSplitter* splitter_;
	QxControl* handle_;
	QString handleTextRecent_, handleTextBookmarks_;
	QxControl* bottomStack_;
	QStackedLayout* bottomStackLayout_;
	QBoxLayout* bottomToolLayout_;
	
	QxUrlListModel* recentModel_;
	QxUrlListView* recentView_;
	QMenu* recentContextMenu_;
	
	QxUrlListModel* bookmarksModel_;
	QxUrlListView* bookmarksView_;
	QMenu* bookmarksContextMenu_;
	QAction* bookmarksShowFullPathAction_;
};

} // namespace pacu

#endif // PACU_QXFILEBROWSER_HPP
