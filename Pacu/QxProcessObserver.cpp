#include "QxProcessObserver.hpp"

namespace pacu
{

QxProcessObserver::QxProcessObserver(Ref<Process> process, QObject* parent)
	: QThread(parent),
	  process_(process),
	  exitCode_(0)
{}

int QxProcessObserver::exitCode() const
{
	return exitCode_;
}

void QxProcessObserver::run()
{
	exitCode_ = process_->wait();
	emit terminated();
}

} // namespace pacu
