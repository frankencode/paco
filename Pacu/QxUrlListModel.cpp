#include <QDebug>
#include <QDir>
#include <QStyle>
#include <QMimeData>
#include <QApplication>
#include "QxUrlListModel.hpp"

namespace pacu
{

QxUrlListModel::QxUrlListModel(QObject* parent)
	: QAbstractListModel(parent),
	  pathReduction_(-1)
{}

QList<QUrl> QxUrlListModel::urls() const { return urls_; }

void QxUrlListModel::setUrls(QList<QUrl> urls)
{
	urls_ = urls;
	fileInfos_.clear();
	for (int row = 0; row < urls_.count(); ++row) {
		QUrl url = urls_.at(row);
		if (url.scheme() == "file")
			fileInfos_.insert(row, QFileInfo(url.path()));
	}
	reset();
}

void QxUrlListModel::setLocalPaths(QStringList paths)
{
	QList<QUrl> urls;
	for (int row = 0; row < paths.count(); ++row)
		urls.append(QUrl::fromLocalFile(paths.at(row)));
	setUrls(urls);
}

void QxUrlListModel::appendUnique(QUrl url)
{
	if (!urls_.contains(url)) {
		int row = urls_.count();
		urls_.append(url);
		if (url.scheme() == "file")
			fileInfos_.insert(row, QFileInfo(url.path()));
		/*insertRow(row);
		dataChanged(index(row), index(row));*/
		reset();
	}
}

void QxUrlListModel::remove(QModelIndex index)
{
	if (!index.isValid())
		return;
	urls_.removeAt(index.row());
	reloadFileInfos();
	reset();
}

QStringList QxUrlListModel::save() const
{
	QStringList sl;
	for (int row = 0; row < urls_.count(); ++row)
		sl << urls_.at(row).toString();
	return sl;
}

void QxUrlListModel::load(QStringList urls)
{
	urls_.clear();
	fileInfos_.clear();
	for (int row = 0; row < urls.count(); ++row) {
		QUrl url(urls.at(row));
		urls_.append(url);
		if (url.scheme() == "file")
			fileInfos_.insert(row, QFileInfo(url.path()));
	}
	reset();
}

int QxUrlListModel::pathReduction() const { return pathReduction_; }
void QxUrlListModel::setPathReduction(int value) { pathReduction_ = value; reset(); }

int QxUrlListModel::rowCount(const QModelIndex& parent) const
{
	return urls_.count();
}

QVariant QxUrlListModel::data(const QModelIndex& index, int role) const
{
	QVariant value;
	if (index.column() == 0)
	{
		if (role == Qt::DisplayRole)
		{
			QUrl url = urls_.at(index.row());
			
			if (url.scheme() == "file")
			{
				QString path = url.toLocalFile();
				
				if (pathReduction_ != -1)
				{
					QStringList components;
					QFileInfo info = fileInfos_.value(index.row());
					components << info.fileName();
					QDir dir = info.dir();
					while (components.count() < pathReduction_) {
						components << dir.dirName();
						if (!dir.cdUp()) break;
					}
					
					QStringList reversed;
					for (int i = components.count() - 1; i >= 0; --i)
						reversed << components.at(i);
						
					path = reversed.join(QDir::separator());
				}
				
				value = path;
			}
			else
				value = url.toString();
		}
		else if (role == Qt::DecorationRole)
		{
			QIcon icon;
			if (fileInfos_.contains(index.row()))
				icon = iconProvider_.icon(fileInfos_.value(index.row()));
			else
				icon = iconProvider_.icon(QFileIconProvider::Network);
			if (icon.isNull())
				icon = qApp->style()->standardPixmap(QStyle::SP_MessageBoxCritical);
			value = icon;
		}
	}
	
	return value;
}

Qt::ItemFlags QxUrlListModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);
	
	if (index.isValid())
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
	else
		return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList QxUrlListModel::mimeTypes() const
{
	QStringList sl;
	sl << "text/uri-list";
	return sl;
}

Qt::DropActions QxUrlListModel::supportedDropActions() const
{
	return /*Qt::CopyAction |*/ Qt::MoveAction;
}

QMimeData* QxUrlListModel::mimeData(const QModelIndexList& indexes) const
{
	QMimeData *mimeData = new QMimeData;
	QList<QUrl> l;
	for (int i = 0; i < indexes.count(); ++i)
		l.append(urls_.at(indexes.at(i).row()));
	mimeData->setUrls(l);
	return mimeData;
}

bool QxUrlListModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
	/*qDebug() << "dropMimeData(): row, column, action =" << row << "," << column << "," << action;
	qDebug() << "                parent.row(), parent.isValid() =" << parent.row() << "," << parent.isValid();*/
	
	if (!data->hasFormat("text/uri-list"))
		return false;
	
	if (action == Qt::IgnoreAction)
	{}
	else if (action == Qt::MoveAction)
	{
		if (row == -1) {
			if (parent.isValid())
				row = parent.row();
			else
				row = rowCount() - 1;
		}
		
		if (row >= rowCount())
			row = rowCount() - 1;
		
		QList<QUrl> l = data->urls();
		if (l.count() != 1) return false;
		QUrl url = l.at(0);
		int row0 = urls_.indexOf(url);
		
		if ((row0 == -1) || (row == -1))
			return false;
		
		// qDebug() << "move:" << row0 << "=>" << row;
		
		urls_.move(row0, row);
		reloadFileInfos();
		return true;
	}
	
	return false;
}

void QxUrlListModel::reloadFileInfos()
{
	fileInfos_.clear();
	for (int row = 0; row < urls_.count(); ++row) {
		QUrl url = urls_.at(row);
		if (url.scheme() == "file") {
			QFileInfo info(url.path());
			if (info.exists())
				fileInfos_.insert(row, info);
		}
	}
}

} // namespace pacu
