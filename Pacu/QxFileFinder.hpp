#ifndef PACU_QXFILEFINDER_HPP
#define PACU_QXFILEFINDER_HPP

#include <QThread>
#include <QFileInfo>
#include <QStringList>

namespace pacu
{

class QxFileFinder: public QThread
{
	Q_OBJECT
	
public:
	QxFileFinder(QObject* parent = 0);
	
public slots:
	void startSearch(QString dirPath, QString pattern, int maxResults = 11);
	void stopSearch();
	
signals:
	void found(QStringList paths);
	
private:
	virtual void run();
	void search(QString dirPath);
	
	QString dirPath_;
	QStringList pattern_;
	int numResults_, maxResults_;
	bool stop_;
	QStringList results_;
	int depth_;
};

} // namespace pacu

#endif // PACU_QXFILEFINDER_HPP
