#include "QxFileSystemWatcher.hpp"

namespace pacu
{

QxFileSystemWatcher::QxFileSystemWatcher(QObject* parent)
	: QObject(parent),
	  fileSystemWatcher_(new QFileSystemWatcher(this))
{
	connect(fileSystemWatcher_, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChangedFilter(const QString&)));
}

void QxFileSystemWatcher::addPath(QString path)
{
	lastModified_.insert(path, QFileInfo(path).lastModified());
	fileSystemWatcher_->addPath(path);
}

void QxFileSystemWatcher::removePath(QString path)
{
	lastModified_.remove(path);
	fileSystemWatcher_->removePath(path);
}

void QxFileSystemWatcher::fileChangedFilter(const QString& path)
{
	QDateTime lm = QFileInfo(path).lastModified();
	if (lastModified_.value(path) != lm) {
		lastModified_.remove(path);
		lastModified_.insert(path, lm);
		emit fileChanged(path);
	}
}

} // namespace pacu
