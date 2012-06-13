#ifndef PACU_SIGNALADAPTER_HPP
#define PACU_SIGNALADAPTER_HPP

#include <QDebug>
#include <QObject>
#include <ftl/Event.hpp>
#include "UseFtl.hpp"

namespace pacu
{

class SignalAdapter: public QObject, public Action
{
	Q_OBJECT
	
public:
	SignalAdapter(QObject* parent = 0): QObject(parent) {}
	
signals:
	void emitted();

private:
	virtual void run() { emit emitted(); }
};

} // namespace pacu

#endif // PACU_SIGNALADAPTER_HPP
