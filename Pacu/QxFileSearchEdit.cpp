#include <QApplication>
#include <QDir>
#include <QEvent>
#include <QTimer>
#include <QFileIconProvider>
#include <QKeyEvent>
// #include <QFocusEvent>
#include <QDebug> // DEBUG
#include "QxFileFinder.hpp"
#include "QxFileSearchItem.hpp"
#include "QxFileSearchEdit.hpp"

namespace pacu
{

QxFileSearchEdit::QxFileSearchEdit(QWidget* parent, int popupAlign)
	: QxControl(parent, new QxVisual(styleManager()->style("searchEdit"))),
	  finder_(new QxFileFinder(this))
{
	setMode(QxControl::LineEditMode);
	setFocusPolicy(Qt::StrongFocus);
	visual()->setText("");
	visual()->setDescription(QDir::currentPath() + tr("/<file name>"));
	
	setFixedHeight(sizeHint().height());
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	connect(this, SIGNAL(textEdited(QString)), this, SLOT(lookup(QString)));
	connect(this, SIGNAL(submitted()), this, SLOT(openFile()));
	connect(finder_, SIGNAL(found(QStringList)), this, SLOT(found(QStringList)));
	connect(finder_, SIGNAL(finished()), this, SLOT(finished()));

	results_ = new QxListPopup(this, popupAlign);
	results_->setWindowOpacity(0.9); // style HACK
	connect(this, SIGNAL(pressed()), results_, SLOT(hide()));
	connect(this, SIGNAL(submitted()), results_, SLOT(hide()));
	connect(this, SIGNAL(dismissed()), this, SLOT(clearFocus()));
	connect(this, SIGNAL(dismissed()), this, SLOT(clearText()));
	connect(this, SIGNAL(dismissed()), results_, SLOT(hide()));
	connect(this, SIGNAL(rotate()), results_, SLOT(backRotateSelection()));
	connect(this, SIGNAL(backRotate()), results_, SLOT(rotateSelection()));
	connect(this, SIGNAL(rotate()), this, SLOT(unselect()));
	connect(this, SIGNAL(backRotate()), this, SLOT(unselect()));
	connect(this, SIGNAL(doubleClicked()), this, SLOT(selectAll()));
	
	responseTimer_ = new QTimer(this);
	responseTimer_->setInterval(250);
	connect(responseTimer_, SIGNAL(timeout()), results_, SLOT(show()));
	connect(results_, SIGNAL(stopResponseTimer()), responseTimer_, SLOT(stop()));
	connect(this, SIGNAL(dismissed()), responseTimer_, SLOT(stop()));
}

QxFileSearchEdit::~QxFileSearchEdit()
{
	if (finder_->isRunning()) {
		finder_->stopSearch();
		finder_->wait();
	}
}

void QxFileSearchEdit::setCurrentDir(QString path)
{
	visual()->setText("");
	visual()->setDescription(QDir(path).dirName() + tr("/<file name>"));
	results_->hide();
	results_->itemList()->clear();
}

void QxFileSearchEdit::lookup(QString prefix)
{
	if (finder_->isRunning()) {
		finder_->stopSearch();
		finder_->wait();
	}
	
	QApplication::processEvents(QEventLoop::ExcludeUserInputEvents); // eat up a possibly pending search result
	
	results_->hide();
	results_->itemList()->clear();
	QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	
	if (prefix.length() > 0) {
		finder_->startSearch(QDir::currentPath(), prefix);
		// if (!responseTimer_->isActive())
		responseTimer_->start();
		QTimer::singleShot(500, this, SLOT(waiting()));
	}
}

void QxFileSearchEdit::waiting()
{
	if (finder_->isRunning())
		QApplication::setOverrideCursor(Qt::WaitCursor);
}

void QxFileSearchEdit::found(QStringList paths)
{
	if (visual()->text().length() == 0) return; // sync HACK
	
	// autoComplete(QFileInfo(paths.at(0)).fileName());
	
	QFileIconProvider iconProvider;
	for (int i = 0; i < paths.count(); ++i) {
		QString path = paths.at(i);
		QFileInfo fileInfo(path);
		QPixmap icon = iconProvider.icon(fileInfo).pixmap(QSize(16, 16));
		QxControl* item = new QxFileSearchItem(this, icon, fileInfo.fileName(), path);
		connect(item, SIGNAL(requestSelection()), results_, SLOT(unselectAll()));
		connect(item, SIGNAL(selected(QString)), this, SLOT(autoComplete(QString)));
		connect(item, SIGNAL(pressed()), this, SLOT(openFile()));
		results_->itemList()->append(item);
	}
	
	/*if (paths.count() > 0)
		results_->show();
	else*/
		results_->hide();
}

void QxFileSearchEdit::finished()
{
	while (QApplication::overrideCursor())
		QApplication::restoreOverrideCursor();
}

void QxFileSearchEdit::autoComplete(QString fileName)
{
	if (hasSelection())
		deleteSelection();
	QString prefix = text();
	int cursorPosSaved = cursorPos();
	if (prefix.length() < fileName.length()) {
		for (int i = 0, n = prefix.length(); i < n; ++i) {
			if (prefix[i] != fileName[i]) {
				if (prefix[i].toUpper() == fileName[i])
					prefix[i] = prefix[i].toUpper();
				else if (prefix[i].toLower() == fileName[i])
					prefix[i] = prefix[i].toLower();
			}
		}
		if (prefix == fileName.mid(0, prefix.length())) {
			setText(fileName, cursorPosSaved);
			setSelection(prefix.length(), fileName.length());
		}
	}
}

void QxFileSearchEdit::openFile()
{
	QxFileSearchItem* item = qobject_cast<QxFileSearchItem*>(results_->selectedItem());
	if (item) {
		emit openFile(item->details());
		clearText();
	}
	else {
		QString name = text().toLower();
		for (int i = 0, n = results_->itemList()->length(); i < n; ++i) {
			QxFileSearchItem* item = qobject_cast<QxFileSearchItem*>(results_->itemList()->get(i));
			QString candidate = item->details();
			if (QFileInfo(candidate).fileName().toLower() == name) {
				emit openFile(candidate);
				clearText();
				break;
			}
		}
	}
}

void QxFileSearchEdit::keyPressEvent(QKeyEvent* event)
{
	if ((event->key() == Qt::Key_Space) && (results_->selectedItem()))
		emit submitted();
	else
		QxControl::keyPressEvent(event);
}

void QxFileSearchEdit::focusInEvent(QFocusEvent* event)
{
	if ((results_->itemList()->length() > 0) && (event->reason() != Qt::MouseFocusReason)) {
		results_->show();
		selectAll();
	}
	QxControl::focusInEvent(event);
}

void QxFileSearchEdit::focusOutEvent(QFocusEvent* event)
{
	//if (results_->isVisible())
	results_->hide();
	responseTimer_->stop();
	QxControl::focusOutEvent(event);
}

} // namespace pacu
