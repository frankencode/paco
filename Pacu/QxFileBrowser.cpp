#include <QStyle>
#ifdef Q_WS_MAC
#include <QMacStyle>
#include <QProxyStyle>
#endif
#include <QFileSystemModel>
#include <QDesktopServices>
#include <QLayout>
#include <QStackedLayout>
#include <QSettings>
#include <QHeaderView>
#include <QTextStream>
#include <QTimer>
#include <QApplication>
#include <QFileDialog>
#include <QDockWidget>
#include <QDebug> // DEBUG
#include <vide/QxStyleManager.hpp>
#include "QxFileBrowser.hpp"

namespace pacu
{

QxFileBrowser::QxFileBrowser(QWidget* parent)
	: QWidget(parent),
	  gotoMenu_(0),
	  recentMenu_(0),
	  showHidden_(false),
	  showDetails_(true),
	  createFileDialog_(0),
	  createDirDialog_(0),
	  renameDialog_(0)
{
	//--------------------------------------------------------------------------
	// setup primary model and view
	
	cwdModel_ = new QFileSystemModel(this);
	cwdModel_->setReadOnly(false);
	QModelIndex root = cwdModel_->setRootPath(QDir::currentPath());
	connect( cwdModel_, SIGNAL(fileRenamed(const QString&, const QString&, const QString&)),
	         this, SLOT(fileRenamed(const QString&, const QString&, const QString&)) );
	
	dirView_ = new QxFileSystemView(this);
	dirView_->setModel(cwdModel_);
	dirView_->header()->hide();
	dirView_->setRootIndex(root);
	dirView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	dirView_->setDragDropMode(QAbstractItemView::DragOnly);
	connect(dirView_, SIGNAL(activated(const QModelIndex&)), this, SLOT(cwdSet(const QModelIndex&)));
	connect(dirView_, SIGNAL(escape()), this, SIGNAL(escape()));
	connect(dirView_, SIGNAL(cdUp()), this, SLOT(cdUp()));
	connect(cwdModel_, SIGNAL(layoutChanged()), dirView_, SLOT(resizeColumnsToContents()));
	
	setFocusProxy(dirView_);
	
	// dirView_->setTextElideMode(Qt::ElideMiddle);
	dirView_->setFrameStyle(QFrame::NoFrame);
	// dirView_->setIndentation(18); // together with a branch pixmap could improve branch alignment
	dirView_->setLineWidth(0);
	dirView_->setMidLineWidth(0);
	#ifdef Q_WS_MAC
	dirView_->setStyleSheet(
		"QTreeView {"
		"  font-size: 12px;"
		"}"
		"QTreeView::item {"
		"  padding-top: 1px;"
		"}"
	);
	#endif
	{
		QPalette pal = dirView_->palette();
		pal.setColor(QPalette::Base, styleManager()->color("fileBrowserDirViewBgColor"));
		dirView_->setPalette(pal);
	}
	
	//--------------------------------------------------------------------------
	// setup navigation bar / panel head
	
	gotoButton_ = new QxControl(this, new QxVisual(styleManager()->style("fileBrowserGotoButton")));
	gotoButton_->setMode(QxControl::MenuMode);
	gotoButton_->visual()->setText(QDir::current().dirName());
	gotoButton_->setMinimumWidth(10); // design HACK
	gotoButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	{
		QPixmap folderIcon = QFileIconProvider().icon(QFileInfo(QDir::currentPath())).pixmap(QSize(16, 16));
		gotoButton_->visual()->setLeadingIcon(folderIcon);
	}
	#ifdef Q_WS_MAC
	#ifdef QT_MAC_USE_COCOA
	connect(gotoButton_, SIGNAL(toggled(bool)), this, SLOT(blockFloating(bool)));
	#endif // QT_MAC_USE_COCOA
	#endif // Q_WS_MAC
	
	updateGotoMenu();
	
	class QxCdUpButton: public QxControl {
	public:
		QxCdUpButton(QWidget* parent)
			: QxControl(parent, new QxVisual(styleManager()->style("fileBrowserCdUpButton")))
		{}
	private:
		// workaround HACK, prevent undocking, map double click event to single click
		virtual void mouseDoubleClickEvent(QMouseEvent* event) { press(); event->accept(); }
	};
	QxControl* cdUpButton = new QxCdUpButton(this);
	cdUpButton->setMode(QxControl::TouchMode);
	connect(cdUpButton, SIGNAL(pressed()), this, SLOT(cdUp()));
	
	//--------------------------------------------------------------------------
	// define context actions
	
	contextMenu_ = new QMenu(this);
	
	// QxDesignHack::beautify(contextMenu_);
	openDefaultAction_ = new QAction(tr("Open in Default App."), this);
	openDefaultAction_->setShortcut(tr("Ctrl+D"));
	openDefaultAction_->setShortcutContext(Qt::WidgetShortcut);
	openFileAction_ = new QAction(tr("Open in %1").arg(qApp->applicationName()), this);
	
	createFileAction_ = contextMenu_->addAction(tr("New File"));
	createDirAction_ = contextMenu_->addAction(tr("New Folder"));
	contextMenu_->addSeparator();
	contextMenu_->addAction(openFileAction_);
	contextMenu_->addAction(openDefaultAction_);
	contextMenu_->addSeparator();
	renameAction_ = contextMenu_->addAction(tr("Rename"));
	deleteAction_ = contextMenu_->addAction(tr("Move To Trash"));
	contextMenu_->addSeparator();
	bookmarkAction_ = contextMenu_->addAction(tr("Bookmark"));
	contextMenu_->addSeparator();
	showHiddenAction_ = contextMenu_->addAction(tr("Show Hidden"));
	showDetailsAction_ = contextMenu_->addAction(tr("Show Details"));
	
	connect(createFileAction_, SIGNAL(triggered()), this, SLOT(createFile()));
	connect(createDirAction_, SIGNAL(triggered()), this, SLOT(createDir()));
	connect(openDefaultAction_, SIGNAL(triggered()), this, SLOT(openDefault()));
	connect(openFileAction_, SIGNAL(triggered()), this, SLOT(openFile()));
	connect(renameAction_, SIGNAL(triggered()), this, SLOT(rename()));
	connect(deleteAction_, SIGNAL(triggered()), this, SLOT(delete_()));
	connect(dirView_, SIGNAL(delKeyPressed()), this, SLOT(delete_()));
	connect(showHiddenAction_, SIGNAL(toggled(bool)), this, SLOT(showHidden(bool)));
	connect(showDetailsAction_, SIGNAL(toggled(bool)), this, SLOT(showDetails(bool)));
	connect(bookmarkAction_, SIGNAL(triggered()), this, SLOT(bookmark()));
	
	showHiddenAction_->setCheckable(true);
	showDetailsAction_->setCheckable(true);
	showHiddenAction_->setChecked(false);
	showDetailsAction_->setChecked(false);
	
	//--------------------------------------------------------------------------
	// setup toolbar
	
	QxControl* plusButton = new QxControl(this, new QxVisual(styleManager()->style("fileBrowserPlusButton")));
	plusButton->setMode(QxControl::TouchMode);
	connect(plusButton, SIGNAL(pressed()), this, SLOT(createFile()));
	
	QxControl* wheelButton = new QxControl(this, new QxVisual(styleManager()->style("fileBrowserWheelButton")));
	wheelButton->setMenu(contextMenu_);
	wheelButton->setMenuPopupMode(QxControl::DownsideMenuPopup|QxControl::UpsideMenuPopup|QxControl::PreferUpsideMenuPopup);
	
	QxControl* recentButton = new QxControl(this, new QxVisual(styleManager()->style("fileBrowserRecentButton")));
	recentButton->setMode(QxControl::TouchMode);
	connect(recentButton, SIGNAL(pressed()), this, SLOT(recentShowOrHide()));
	
	QxControl* bookmarksButton = new QxControl(this, new QxVisual(styleManager()->style("fileBrowserBookmarksButton")));
	bookmarksButton->setMode(QxControl::TouchMode);
	connect(bookmarksButton, SIGNAL(pressed()), this, SLOT(bookmarksShowOrHide()));
	
	statusBar_ = new QxStatusBar(this);
	
	showHidden(false);
	showDetails(false);
	
	//--------------------------------------------------------------------------
	// setup bottom views
	
	recentModel_ = new QxUrlListModel(this);
	
	recentView_ = new QxUrlListView(this);
	// recentView_->header()->hide();
	// recentView_->setViewMode(QListView::IconMode);
	// recentView_->setIconSize(QSize(16, 16));
	recentView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	recentView_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	recentView_->setTextElideMode(Qt::ElideLeft);
	recentView_->setFrameStyle(QFrame::NoFrame);
	recentView_->setLineWidth(0);
	recentView_->setMidLineWidth(0);
	recentView_->setStyleSheet(
		"QListView::item {"
		"  padding-top: 1px;"
		"}"
	);
	// recentView_->setAlternatingRowColors(true);
	recentView_->setModel(recentModel_);
	connect(recentView_, SIGNAL(activated(const QModelIndex&)), this, SLOT(gotoRecent(const QModelIndex&)));
	
	recentContextMenu_ = new QMenu(this);
	recentContextMenu_->addAction(tr("Goto"), this, SLOT(gotoRecent()));
	recentContextMenu_->addAction(tr("Add To Bookmarks"), this, SLOT(recentAddToBookmarks()));
	
	bookmarksModel_ = new QxUrlListModel(this);
	bookmarksModel_->setPathReduction(1);
	
	bookmarksView_ = new QxUrlListView(this);
	bookmarksView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	bookmarksView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	bookmarksView_->setTextElideMode(Qt::ElideLeft);
	bookmarksView_->setFrameStyle(QFrame::NoFrame);
	bookmarksView_->setLineWidth(0);
	bookmarksView_->setMidLineWidth(0);
	bookmarksView_->setStyleSheet(
		"QListView::item {"
		"  padding-top: 1px;"
		"}"
	);
	bookmarksView_->setModel(bookmarksModel_);
	bookmarksView_->setDragDropMode(QAbstractItemView::InternalMove);
	bookmarksView_->setDragEnabled(true);
	bookmarksView_->setAcceptDrops(true);
	bookmarksView_->setDropIndicatorShown(true);
	connect(bookmarksView_, SIGNAL(activated(const QModelIndex&)), this, SLOT(gotoBookmark(const QModelIndex&)));
	
	bookmarksContextMenu_ = new QMenu(this);
	bookmarksContextMenu_->addAction(tr("Add Current Directory"), this, SLOT(cwdAddToBookmark()));
	bookmarksContextMenu_->addAction(tr("Add Other Directory"), this, SLOT(addDirBookmark()));
	bookmarksContextMenu_->addSeparator();
	bookmarksContextMenu_->addAction(tr("Goto Here"), this, SLOT(gotoBookmark()));
	bookmarksContextMenu_->addAction(tr("Remove From List"), this, SLOT(removeBookmark()));
	bookmarksContextMenu_->addSeparator();
	bookmarksShowFullPathAction_ = bookmarksContextMenu_->addAction(tr("Show Full Path"));
	bookmarksShowFullPathAction_->setCheckable(true);
	bookmarksShowFullPathAction_->setChecked(bookmarksModel_->pathReduction() == -1);
	connect(bookmarksShowFullPathAction_, SIGNAL(toggled(bool)), this, SLOT(bookmarksShowFullPath(bool)));
	
	#ifdef Q_WS_MAC
	{
		QProxyStyle* proxyStyle = qobject_cast<QProxyStyle*>(style());
		QMacStyle* macStyle = qobject_cast<QMacStyle*>((proxyStyle) ? proxyStyle->baseStyle() : style());
		if (macStyle) {
			macStyle->setFocusRectPolicy(dirView_, QMacStyle::FocusDisabled);
			macStyle->setFocusRectPolicy(recentView_, QMacStyle::FocusDisabled);
			macStyle->setFocusRectPolicy(dirView_, QMacStyle::FocusDisabled);
			macStyle->setFocusRectPolicy(bookmarksView_, QMacStyle::FocusDisabled);
		}
	}
	#endif
	
	//--------------------------------------------------------------------------
	// layout widgets
	
	handle_ = new QxControl(this, new QxVisual(styleManager()->style("fileBrowserSplitter")));
	handle_->visual()->setText("");
	handleTextRecent_ = tr("Recent Places");
	handleTextBookmarks_ = tr("Bookmarks");
	
	bottomStack_ = new QxControl(this);
	bottomStackLayout_ = new QStackedLayout;
	bottomStackLayout_->addWidget(recentView_);
	bottomStackLayout_->addWidget(bookmarksView_);
	bottomStack_->setLayout(bottomStackLayout_);
	bottomStack_->setVisible(false);
	
	splitter_ = new QxSplitter(this);
	splitter_->setOrientation(Qt::Vertical);
	splitter_->setHandle(1, handle_);
	splitter_->setHandleWidth(styleManager()->constant("fileBrowserSplitterWidth"));
	splitter_->addWidget(dirView_);
	/*{
		QxControl* carrier = new QxControl(this, new QxVisual(styleManager()->style("fileBrowserDirView")));
		QGridLayout* layout = new QGridLayout;
		layout->setSpacing(0);
		layout->setMargin(0);
		layout->addWidget(dirView_);
		carrier->setLayout(layout);
		splitter_->addWidget(carrier);
	}*/
	splitter_->addWidget(bottomStack_);
	
	// make dirView_ grow/shrink dynamically, while bottomStack_ keeps user-defined size
	splitter_->setStretchFactor(0, 1);
	splitter_->setStretchFactor(1, 0);
	
	QDockWidget* dock = qobject_cast<QDockWidget*>(parent);
	if (dock) {
		dock->setWidget(this);
		// connect(dock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(adaptToDockLocation(Qt::DockWidgetArea)));
	}
	
	QVBoxLayout* col = new QVBoxLayout;
	col->setSpacing(0);
	col->setMargin(0);
	{
		QxControl* carrier = new QxControl(parent, new QxVisual(styleManager()->style("fileBrowserNavCarrier")));
		QHBoxLayout* row = new QHBoxLayout;
		row->setSpacing(0);
		row->setMargin(0);
		row->addWidget(gotoButton_);
		row->addStretch();
		row->addWidget(cdUpButton);
		carrier->setLayout(row);
		
		if (dock)
			dock->setTitleBarWidget(carrier);
		else
			col->addWidget(carrier);
	}
	col->addWidget(splitter_);
	col->addWidget(styleManager()->hl(this));
	{
		QBoxLayout* row = new QBoxLayout(QBoxLayout::LeftToRight);
		row->setSpacing(0);
		row->setMargin(0);
		row->addWidget(plusButton);
		row->addWidget(styleManager()->vl(this));
		row->addWidget(wheelButton);
		row->addWidget(styleManager()->vl(this));
		row->addWidget(recentButton);
		row->addWidget(styleManager()->vl(this));
		row->addWidget(bookmarksButton);
		row->addWidget(styleManager()->vl(this));
		row->addWidget(statusBar_);
		col->addLayout(row);
		
		bottomToolLayout_ = row;
	}
	setLayout(col);
}

void QxFileBrowser::saveState(QSettings* settings)
{
	settings->setValue("fileBrowser/currentPath", QDir::currentPath());
	settings->setValue("fileBrowser/cwdPast", cwdPast_);
	settings->setValue("fileBrowser/cwdFuture", cwdFuture_);
	// settings->setValue("fileBrowser/isVisible", isVisible());
	settings->setValue("fileBrowser/showHidden", showHiddenAction_->isChecked());
	settings->setValue("fileBrowser/showDetails", showDetailsAction_->isChecked());
	settings->setValue("fileBrowser/splitter", splitter_->saveState());
	settings->setValue("fileBrowser/recentView/isVisible", recentView_->isVisible());
	settings->setValue("fileBrowser/bookmarksView/isVisible", bookmarksView_->isVisible());
	settings->setValue("fileBrowser/bookmarksModel", bookmarksModel_->save());
	settings->setValue("fileBrowser/bookmarksShowFullPath", bookmarksShowFullPathAction_->isChecked());
}

void QxFileBrowser::restoreState(const QSettings* settings)
{
	if (settings->contains("fileBrowser/currentPath"))
		cwdSet(settings->value("fileBrowser/currentPath").toString());
	if (settings->contains("fileBrowser/cwdPast"))
		cwdPast_ = settings->value("fileBrowser/cwdPast").toStringList();
	if (settings->contains("fileBrowser/cwdFuture"))
		cwdFuture_ = settings->value("fileBrowser/cwdFuture").toStringList();
	//if (settings->contains("fileBrowser/isVisible"))
		//setVisible(settings->value("fileBrowser/isVisible").toBool());
	if (settings->contains("fileBrowser/showHidden"))
		showHiddenAction_->setChecked(settings->value("fileBrowser/showHidden").toBool());
	if (settings->contains("fileBrowser/showDetails"))
		showDetailsAction_->setChecked(settings->value("fileBrowser/showDetails").toBool());
	if (settings->contains("fileBrowser/splitter"))
		splitter_->restoreState(settings->value("fileBrowser/splitter").toByteArray());
	bottomStack_->setVisible(false);
	if (settings->contains("fileBrowser/recentView/isVisible")) {
		if (settings->value("fileBrowser/recentView/isVisible").toBool()) {
			handle_->visual()->setText(handleTextRecent_);
			bottomStack_->setVisible(true);
			bottomStackLayout_->setCurrentWidget(recentView_);
		}
	}
	if (settings->contains("fileBrowser/bookmarksView/isVisible")) {
		if (settings->value("fileBrowser/bookmarksView/isVisible").toBool()) {
			handle_->visual()->setText(handleTextBookmarks_);
			bottomStack_->setVisible(true);
			bottomStackLayout_->setCurrentWidget(bookmarksView_);
		}
	}
	if (settings->contains("fileBrowser/bookmarksModel"))
		bookmarksModel_->load(settings->value("fileBrowser/bookmarksModel").toStringList());
	if (settings->contains("fileBrowser/bookmarksShowFullPath"))
		bookmarksShowFullPathAction_->setChecked(settings->value("fileBrowser/bookmarksShowFullPath").toBool());
		
	while (cwdPast_.count() > 7)
		cwdPast_.pop_front();
	updateGotoMenu();
	recentModel_->setLocalPaths(cwdPast_);
}

QList<QUrl> QxFileBrowser::bookmarkedDirs() const { return bookmarksModel_->urls(); }

void QxFileBrowser::openDefault()
{
	QModelIndex index = dirView_->currentIndex();
	if (!index.isValid()) return;
	QDesktopServices::openUrl("file://" + cwdModel_->filePath(index));
}

void QxFileBrowser::openFile()
{
	QModelIndex index = dirView_->currentIndex();
	if (!index.isValid()) return;
	cwdSet(index);
}

void QxFileBrowser::cwdSet(const QString& newPath)
{
	QString path = newPath;
	if ((path.length() > 0) && (path.at(path.length() - 1) == QDir::separator()))
		path = path.left(path.length() - 1);
	QModelIndex root = cwdModel_->setRootPath(path);
	dirView_->setRootIndex(root);
	// updateStatusDisplay();
	QPixmap folderIcon = QFileIconProvider().icon(QFileInfo(newPath)).pixmap(QSize(16, 16));
	QString dirName = QDir(path).dirName();
	gotoButton_->visual()->setText(dirName);
	gotoButton_->visual()->setLeadingIcon(folderIcon);
	// dirView_->setCurrentIndex(cwdModel_->index(0, 0));
	QDir::setCurrent(path);
	emit cwdChanged(QDir::currentPath());
	updateGotoMenu();
	update();
}

void QxFileBrowser::cwdSet(QAction* action)
{
	QString newPath;
	if (action)
		newPath = actionToPath_[action];
	if (newPath != QString()) {
		cwdSet(newPath);
	}
}

void QxFileBrowser::cwdSet(const QModelIndex& index)
{
	QFileInfo info(cwdModel_->filePath(index));
	if (info.isDir()) {
		cwdSet(info.filePath());
	}
	else {
		emit openFile(info.filePath());
		if (isVisible())
			dirView_->setFocus();
		if (!cwdPast_.contains(QDir::currentPath())) {
			while (cwdPast_.count() > 7)
				cwdPast_.pop_front();
			cwdPast_.append(QDir::currentPath());
			cwdFuture_.clear();
			recentModel_->setLocalPaths(cwdPast_);
		}
		updateGotoMenu();
	}
}

void QxFileBrowser::cdUp()
{
	QDir dir = QDir::current();
	if (dir.cdUp())
		cwdSet(dir.path());
}

void QxFileBrowser::showHidden(bool on)
{
	if (showHidden_ != on) {
		showHidden_ = on;
		if (on)
			cwdModel_->setFilter(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System|QDir::Hidden|QDir::Readable|QDir::Writable|QDir::Executable);
		else
			cwdModel_->setFilter(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::Readable|QDir::Writable|QDir::Executable);
		// updateStatusDisplay();
	}
}

void QxFileBrowser::showDetails(bool on)
{
	if (showDetails_ != on) {
		showDetails_ = on;
		if (on) {
			for (int k = 1; k < cwdModel_->columnCount(); ++k)
				dirView_->setColumnHidden(k, false);
		}
		else {
			for (int k = 1; k < cwdModel_->columnCount(); ++k)
				dirView_->setColumnHidden(k, true);
		}
		dirView_->resizeColumnsToContents();
	}
}

void QxFileBrowser::updateStatusDisplay()
{
	int n = cwdModel_->rowCount(dirView_->rootIndex());
	int nf = 0;
	quint64 sz = 0;
	for (int i = 0; i < n; ++i) {
		QModelIndex index = cwdModel_->index(i, 0, dirView_->rootIndex());
		if (!cwdModel_->isDir(index)) {
			sz += QFileInfo(cwdModel_->filePath(index)).size();
			++nf;
		}
	}
	QString s;
	QTextStream out(&s);
	out << nf << tr(" files, ") << (n - nf) << tr(" dirs, ") << sz / 1024 << " KB";
	out.flush();
	statusBar_->display(QxStatusBar::Left, s);
}

void QxFileBrowser::keyPressEvent(QKeyEvent* event)
{
	QItemSelectionModel* sm = dirView_->selectionModel();
	if ((event->key() == Qt::Key_D) && ((event->modifiers() & Qt::ControlModifier) != 0)) { // HACK
		openDefault();
		event->accept();
		return;
	}
	else if ((event->key() == Qt::Key_Return) && (sm->hasSelection())) {
		QModelIndexList l = sm->selectedRows();
		for (int i = 0; i < l.count(); ++i)
			cwdSet(l[i]);
		dirView_->setFocus();
	}
	QWidget::keyPressEvent(event);
}

void QxFileBrowser::newNameChanged(QString s)
{
	newName_ = s;
}

// dirty harry HACK for inline file creation
void QxFileBrowser::createFilePolling()
{
	QPersistentModelIndex index = cwdModel_->index(newFilePath_);
	if (index.isValid() && (index.flags() & Qt::ItemIsEditable)) {
		cwdModel_->sort(0);
		dirView_->setCurrentIndex(index);
		dirView_->edit(index);
		newFilePath_ = QString();
	}
	else {
		QTimer::singleShot(100, this, SLOT(createFilePolling()));
	}
}

void QxFileBrowser::createFile()
{
	QDir dir = cwdModel_->rootDirectory();
	if (dirView_->currentIndex().isValid()) {
		if (cwdModel_->isDir(dirView_->currentIndex()))
			dir = QDir(cwdModel_->filePath(dirView_->currentIndex()));
	}
	if (!dir.exists()) return;
	QString prefix = "noname";
	QString suffix = "txt";
	QString newName = prefix + "." + suffix;
	int i = 1;
	while (QFileInfo(dir.filePath(newName)).exists()) {
		newName = QString("%1_%2.%3").arg(prefix).arg(i).arg(suffix);
		++i;
	}
	
	newFilePath_ = dir.filePath(newName);
	QFile newFile(dir.filePath(newFilePath_));
	if (newFile.open(QIODevice::ReadWrite)) {
		newFile.close();
		QTimer::singleShot(0, this, SLOT(createFilePolling()));
	}
}

void QxFileBrowser::createDir()
{
	QDir dir = cwdModel_->rootDirectory();
	if (dirView_->currentIndex().isValid()) {
		if (cwdModel_->isDir(dirView_->currentIndex()))
			dir = QDir(cwdModel_->filePath(dirView_->currentIndex()));
	}
	if (!dir.exists()) return;
	QString prefix = "noname";
	QString newName = prefix;
	int i = 1;
	while (QFileInfo(dir.filePath(newName)).exists()) {
		newName = QString("%1_%2").arg(prefix).arg(i);
		++i;
	}
	
	if (dir.mkdir(newName)) {
		newFilePath_ = dir.filePath(newName);
		QTimer::singleShot(0, this, SLOT(createFilePolling()));
	}
}

void QxFileBrowser::rename()
{
	dirView_->edit(dirView_->currentIndex());
}

void QxFileBrowser::delete_()
{
	QModelIndex currentIndex = dirView_->currentIndex();
	if (currentIndex.isValid()) {
		QItemSelectionModel* sm = dirView_->selectionModel();
		if (sm->hasSelection()) {
			QModelIndexList l = sm->selectedRows();
			currentIndex = QModelIndex(); // visual HACK, cwdModel_->index(currentIndex.row() + l.count(), 0, currentIndex.parent());
			for (int i = 0; i < l.count(); ++i) {
				QModelIndex mi = l[i];
				QDir dest = QDir::home();
				dest.mkdir(".Trash");
				dest.cd(".Trash");
				QString destBasePath = dest.filePath(cwdModel_->fileName(mi));
				QString destPath = destBasePath;
				int i = 1;
				while (QFileInfo(destPath).exists()) {
					destPath = QString("%1_%2").arg(destBasePath).arg(i);
					++i;
				}
				QFile(cwdModel_->filePath(mi)).rename(destPath);
			}
			dirView_->setCurrentIndex(currentIndex);
		}
	}
}

void QxFileBrowser::fileRenamed(const QString& path, const QString& oldName, const QString& newName )
{
	if (QFileInfo(QDir(path).filePath(oldName)).isDir())
		emit dirRenamed(QDir(path).filePath(oldName), QDir(path).filePath(newName));
	else
		emit fileRenamed(QDir(path).filePath(oldName), QDir(path).filePath(newName));
}

void QxFileBrowser::contextMenuEvent(QContextMenuEvent* event)
{
	QRect cwdViewRect(dirView_->viewport()->mapToGlobal(QPoint(0, 0)), dirView_->viewport()->size());
	QRect recentViewRect(recentView_->viewport()->mapToGlobal(QPoint(0, 0)), recentView_->viewport()->size());
	QRect bookmarksViewRect(bookmarksView_->viewport()->mapToGlobal(QPoint(0, 0)), bookmarksView_->viewport()->size());
	QPoint pos = event->globalPos();
	
	if (cwdViewRect.contains(pos)) {
		QItemSelectionModel* sm = dirView_->selectionModel();
		bool multiSelect = (sm->selectedRows().count() > 1);
		renameAction_->setDisabled(multiSelect);
		bookmarkAction_->setEnabled(cwdModel_->isDir(dirView_->currentIndex()));
		dirView_->ignoreNextFocusOutEvent();
		contextMenu_->exec(pos);
	}
	else if (recentView_->isVisible() && (recentViewRect.contains(pos))) {
		dirView_->ignoreNextFocusOutEvent();
		recentContextMenu_->exec(pos);
	}
	else if (bookmarksView_->isVisible() && (bookmarksViewRect.contains(pos))) {
		dirView_->ignoreNextFocusOutEvent();
		bookmarksContextMenu_->exec(pos);
	}
}

void QxFileBrowser::recentShowOrHide()
{
	if (bottomStack_->isVisible()) {
		if (bottomStackLayout_->currentWidget() == recentView_) {
			bottomStack_->hide();
			return;
		}
	}
	else
		bottomStack_->show();
	bottomStackLayout_->setCurrentWidget(recentView_);
	handle_->visual()->setText(handleTextRecent_);
}

void QxFileBrowser::bookmarksShowOrHide()
{
	if (bottomStack_->isVisible()) {
		if (bottomStackLayout_->currentWidget() == bookmarksView_) {
			bottomStack_->hide();
			return;
		}
	}
	else
		bottomStack_->show();
	bottomStackLayout_->setCurrentWidget(bookmarksView_);
	handle_->visual()->setText(handleTextBookmarks_);
}

void QxFileBrowser::gotoRecent(const QModelIndex& index)
{
	QString path = recentModel_->data(index).toString();
	if (path != QString()) {
		if (QFileInfo(path).exists()) {
			cwdSet(path);
			dirView_->setFocus();
		}
	}
}

void QxFileBrowser::gotoRecent()
{
	gotoRecent(recentView_->currentIndex());
}

void QxFileBrowser::recentAddToBookmarks()
{
	addBookmark(recentModel_->urls().at(recentView_->currentIndex().row()));
}

void QxFileBrowser::gotoBookmark(const QModelIndex& index)
{
	QUrl url = bookmarksModel_->urls().at(index.row());
	if (url.scheme() == "file")
		cwdSet(url.path());
}

void QxFileBrowser::gotoBookmark()
{
	QModelIndex index = bookmarksView_->currentIndex();
	if (index.isValid())
		gotoBookmark(index);
}

void QxFileBrowser::addDirBookmark()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Add Directory To Bookmarks"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontUseSheet | QFileDialog::DontResolveSymlinks);
	if (dir != QString())
		bookmarksModel_->appendUnique(QUrl::fromLocalFile(dir));
}

void QxFileBrowser::cwdAddToBookmark()
{
	bookmarksModel_->appendUnique(QUrl::fromLocalFile(QDir::currentPath()));
}

void QxFileBrowser::bookmark()
{
	QModelIndex index = dirView_->currentIndex();
	if (index.isValid())
		if (cwdModel_->isDir(index))
			bookmarksModel_->appendUnique(QUrl::fromLocalFile(cwdModel_->filePath(index)));
}


void QxFileBrowser::removeBookmark()
{
	bookmarksModel_->remove(bookmarksView_->currentIndex());
	QCoreApplication::processEvents();
}

void QxFileBrowser::bookmarksShowFullPath(bool on)
{
	if (on)
		bookmarksModel_->setPathReduction(-1);
	else
		bookmarksModel_->setPathReduction(1);
}

void QxFileBrowser::blockFloating(bool on)
{
	#ifdef Q_WS_MAC
	#ifdef QT_MAC_USE_COCOA
	QDockWidget* dock = qobject_cast<QDockWidget*>(parentWidget());
	if (dock) {
		QFlags<QDockWidget::DockWidgetFeature> flags = dock->features();
		if (on)
			flags &= ~QDockWidget::DockWidgetMovable;
		else
			flags |= QDockWidget::DockWidgetMovable;
		dock->setFeatures(flags);
	}
	#endif // QT_MAC_USE_COCOA
	#endif // Q_WS_MAC
}

void QxFileBrowser::adaptToDockLocation(Qt::DockWidgetArea area)
{
	if ((area == Qt::LeftDockWidgetArea) && (bottomToolLayout_->direction() != QBoxLayout::LeftToRight))
		bottomToolLayout_->setDirection(QBoxLayout::LeftToRight);
	else if ((area == Qt::RightDockWidgetArea) && (bottomToolLayout_->direction() != QBoxLayout::RightToLeft))
		bottomToolLayout_->setDirection(QBoxLayout::RightToLeft);
}

void QxFileBrowser::addBookmark(QUrl url)
{
	bookmarksModel_->appendUnique(url);
	QCoreApplication::processEvents();
}

void QxFileBrowser::updateGotoMenu()
{
	for (int i = 0, n = cwdPast_.count(); i < n;  ++i) {
		if (!QDir(cwdPast_.at(i)).exists()) {
			cwdPast_.removeAt(i);
			--i;
			--n;
		}
	}
	
	if (gotoMenu_) {
		disconnect(gotoMenu_, 0, 0, 0);
		gotoMenu_->setAttribute(Qt::WA_DeleteOnClose, true); // destruction workaround, HACK
		gotoMenu_->close();
	}
	
	gotoMenu_ = new QMenu(this);
	{
		QFont font = gotoMenu_->font();
		font.setPixelSize(styleManager()->constant("dirOpenMenuFontSizePx"));
		gotoMenu_->setFont(font);
	}
	connect(gotoMenu_, SIGNAL(triggered(QAction*)), this, SLOT(cwdSet(QAction*)));
	actionToPath_.clear();
	
	QFileIconProvider iconProvider;
	QDir dir = QDir::current();
	while (!dir.isRoot()) {
		dir.cdUp();
		QAction* action = gotoMenu_->addAction(iconProvider.icon(QFileInfo(dir.path())), dir.isRoot() ? QDir::separator() : dir.dirName());
		actionToPath_[action] = dir.path();
	}
	
	gotoMenu_->addSeparator();
	
	recentMenu_ = gotoMenu_->addMenu(tr("Recent Places"));
	{
		QFont font = recentMenu_->font();
		font.setPixelSize(styleManager()->constant("dirOpenMenuFontSizePx"));
		recentMenu_->setFont(font);
	}
	// QxDesignHack::beautify(recentMenu_);
	connect(recentMenu_, SIGNAL(triggered(QAction*)), this, SLOT(cwdSet(QAction*)));
	
	int n = cwdPast_.count();
	recentMenu_->setDisabled(n == 0);
	
	for (int i = n-1; i >= 0; --i)
	{
		QString path = cwdPast_[i];
		QString name = path; // QFileInfo(path).fileName();
		if (name == QString()) name = QDir::separator();
		QAction* action = recentMenu_->addAction(iconProvider.icon(QFileInfo(path)), name);
		actionToPath_[action] = path;
	}
	
	gotoButton_->setMenu(gotoMenu_);
}

/*void QxFileBrowser::focusInEvent(QFocusEvent* event)
{
	qDebug() << "QxFileBrowser::focusInEvent(): event->reason() =" << event->reason();
	QWidget::focusInEvent(event);
}*/

} // namespace pacu
