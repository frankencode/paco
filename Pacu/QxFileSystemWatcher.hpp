#ifndef PACU_QXFILESYSTEMWATCHER_HPP
#define PACU_QXFILESYSTEMWATCHER_HPP

#include <QtCore>

namespace pacu
{

class QxFileSystemWatcher: public QObject
{
	Q_OBJECT
	
public:
	QxFileSystemWatcher(QObject* parent = 0);
	void addPath(QString path);
	void removePath(QString path);

signals:
	void fileChanged(const QString& path);

private slots:
	void fileChangedFilter(const QString& path);
	
private:
	QPointer<QFileSystemWatcher> fileSystemWatcher_;
	QMap<QString, QDateTime> lastModified_;
};

} // namespace pacu

#endif // PACU_QXFILESYSTEMWATCHER_HPP
