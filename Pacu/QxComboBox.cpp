#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include "QxComboBox.hpp"

namespace pacu
{

QxComboBox::QxComboBox(QWidget* parent)
	: QComboBox(parent)
{}

/*#ifdef Q_WS_MAC
#ifndef QT_MAC_USE_COCOA
QSize QxComboBox::sizeHint() const { return QComboBox::sizeHint() + QSize(48, 0); }
#endif
#endif*/
/*#ifdef Q_WS_MAC
QSize QxComboBox::minimumSizeHint() const { return QComboBox::minimumSizeHint() + QSize(-12, 0); }
#endif*/

bool QxComboBox::event(QEvent* event)
{
	bool focusHack = false;
	
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		
		#ifdef Q_WS_MAC
		if (keyEvent->key() == Qt::Key_Home) {
			QKeyEvent newKeyEvent(QEvent::KeyPress, Qt::Key_A, keyEvent->modifiers() | Qt::MetaModifier);
			QApplication::sendEvent(this, &newKeyEvent);
		}
		else if (keyEvent->key() == Qt::Key_End) {
			QKeyEvent newKeyEvent(QEvent::KeyPress, Qt::Key_E, Qt::MetaModifier);
			QApplication::sendEvent(this, &newKeyEvent);
		}
		else if ((keyEvent->key() == Qt::Key_Backspace) || (keyEvent->key() == Qt::Key_Delete)) {
			// workaround HACK
			focusHack = true;
		}
		#else
		if ((keyEvent->modifiers() & Qt::ControlModifier) != 0) {
			if (keyEvent->key() == Qt::Key_A) {
				QKeyEvent newKeyEvent(QEvent::KeyPress, Qt::Key_Home, keyEvent->modifiers() ^ Qt::ControlModifier);
				QApplication::sendEvent(this, &newKeyEvent);
			}
			else if (keyEvent->key() == Qt::Key_E) {
				QKeyEvent newKeyEvent(QEvent::KeyPress, Qt::Key_End, keyEvent->modifiers() ^ Qt::ControlModifier);
				QApplication::sendEvent(this, &newKeyEvent);
			}
		}
		#endif
		
		if (keyEvent->key() == Qt::Key_Escape)
			emit escape();
	}
	
	bool ret = QComboBox::event(event);
	
	if (focusHack) { clearFocus(); setFocus(); }
	return ret;
}

} // namespace pacu
