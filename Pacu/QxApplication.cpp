#include <QApplication>
#ifdef Q_WS_MAC
#include <QTimer>
#endif
#include "UseVide.hpp"
#include "QxMainWindow.hpp"
#include "QxApplication.hpp"
#ifdef Q_WS_MAC
#ifndef QT_MAC_USE_COCOA
#include <Carbon/Carbon.h>
#endif
#endif

namespace pacu
{

QxApplication::QxApplication(int& argc, char** argv)
	: QApplication(argc, argv),
	  mainWindow_(0)
{
#ifdef Q_WS_MAC
	setQuitOnLastWindowClosed(false);
#endif
	qxApp = this;
}

QxApplication::~QxApplication()
{
#ifdef Q_WS_MAC
	if (mainWindow_)
		delete mainWindow_;
#endif
}

#ifdef Q_WS_MAC
#ifndef QT_MAC_USE_COCOA
static OSStatus appleEventProcessor(const AppleEvent* ae, AppleEvent* event, long handlerRefCon)
{
	OSType aeClass = typeWildCard;
	OSType aeID = typeWildCard;
	
	AEGetAttributePtr(ae, keyEventClassAttr, typeType, 0, &aeClass, sizeof(aeClass), 0);
	AEGetAttributePtr(ae, keyEventIDAttr, typeType, 0, &aeID, sizeof(aeID), 0);
	
	if ((aeClass == kCoreEventClass) && (aeID == kAEReopenApplication) && (qxApp)) {
		if (qxApp->mainWindow()) {
			qxApp->mainWindow()->reopen();
			return noErr;
		}
	}
	
	return eventNotHandledErr;
}
#endif // QT_MAC_USE_COCOA
#endif // Q_WS_MAC

int QxApplication::exec()
{
	mainWindow_ = new QxMainWindow();
#ifdef Q_WS_MAC
	mainWindow_->setAttribute(Qt::WA_DeleteOnClose, false);
#else
	mainWindow_->setAttribute(Qt::WA_DeleteOnClose, true);
#endif
	mainWindow_->show();
#ifdef Q_WS_MAC
#ifndef QT_MAC_USE_COCOA
	AEEventHandlerUPP appleEventProcessorUPP = AEEventHandlerUPP(appleEventProcessor);
	AEInstallEventHandler(kCoreEventClass, kAEReopenApplication, appleEventProcessorUPP, 0, true);
#endif
#endif
	return QApplication::exec();
}

QxMainWindow* QxApplication::mainWindow() const { return mainWindow_; }

bool QxApplication::event(QEvent* event)
{
	if (event->type() == QEvent::FileOpen) {
		QFileOpenEvent* foe = static_cast<QFileOpenEvent*>(event);
		#ifdef Q_WS_MAC
		#ifndef QT_MAC_USE_COCOA
		if (foe->file() == QString()) {
			if (qxApp->mainWindow())
				qxApp->mainWindow()->reopen();
		}
		else
		#endif
		#endif
		emit fileOpen(foe->file());
		return true;
	}
	
	return QApplication::event(event);
}

QPointer<QxApplication> qxApp = 0;

} // namespace pacu
