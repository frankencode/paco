#ifndef PACU_INTERPOSITIONPROXY_HPP
#define PACU_INTERPOSITIONPROXY_HPP

#include <QObject>
#include <QPointer>
#include <codetips/InterpositionServer.hpp>
#include "UseFtl.hpp"

namespace pacu
{

class QxMainWindow;

class InterpositionProxy: public codetips::InterpositionServer
{
public:
	InterpositionProxy(QxMainWindow* mainWin);
	virtual String redirectOpen(String path);
private:
	QPointer<QxMainWindow> mainWin_;
};

} // namespace pacu

#endif // PACU_INTERPOSITIONPROXY_HPP
