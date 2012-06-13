#ifndef PACU_MONOSPACEFILTER_HPP
#define PACU_MONOSPACEFILTER_HPP

#include <QThread>
#include <QStringList>

namespace pacu
{

class QxMonospaceFilter: public QThread
{
	Q_OBJECT
	
public:
	QxMonospaceFilter(QObject* parent = 0);
	
	void stop();
	
signals:
	void finished(QStringList fontNames);
	
private:
	virtual void run();
	bool stop_;
};

} // namespace pacu

#endif // PACU_MONOSPACEFILTER_HPP
