#ifndef PACU_QXURLLISTMODEL_HPP
#define PACU_QXURLLISTMODEL_HPP

#include <QAbstractListModel>
#include <QUrl>
#include <QFileIconProvider>

namespace pacu
{

class QxUrlListModel: public QAbstractListModel
{
public:
	QxUrlListModel(QObject* parent = 0);
	
	QList<QUrl> urls() const;
	void setUrls(QList<QUrl> urls);
	void setLocalPaths(QStringList paths);
	
	void appendUnique(QUrl url);
	void remove(QModelIndex index);
	
	QStringList save() const;
	void load(QStringList urls);
	
	int pathReduction() const;
	void setPathReduction(int value);
	
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	
	virtual Qt::ItemFlags flags(const QModelIndex& index) const;
	virtual QStringList mimeTypes() const;
	virtual Qt::DropActions supportedDropActions() const;
	virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
	
private:
	QList<QUrl> urls_;
	QMap<int, QFileInfo> fileInfos_;
	QFileIconProvider iconProvider_;
	int pathReduction_;
	
	void reloadFileInfos();
};

} // namespace pacu

#endif //  PACU_QXURLLISTMODEL_HPP
