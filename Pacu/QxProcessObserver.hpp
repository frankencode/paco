#ifndef PACU_QXPROCESSOBSERVER_HPP
#define PACU_QXPROCESSOBSERVER_HPP

#include <QtCore>
#include <ftl/Process.hpp>
#include "UseFtl.hpp"

namespace pacu
{

class QxProcessObserver: public QThread
{
	Q_OBJECT
	
public:
	QxProcessObserver(Ref<Process> process, QObject* parent = 0);
	
	int exitCode() const;

signals:
	void terminated();
	
private:
	void run();
	
	Ref<Process, Owner> process_;
	int exitCode_;
};

} // namespace pacu

#endif // PACU_QXPROCESSOBSERVER_HPP
