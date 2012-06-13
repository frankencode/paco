#include "QxDialog.hpp"

namespace pacu
{

QxDialog::QxDialog(QWidget* parent)
	: QDialog(
#ifdef QT_MAC_USE_COCOA
		0
#else
		parent
#endif
	  )
{
#ifdef Q_WS_MAC
#ifndef QT_MAC_USE_COCOA
	setWindowFlags((windowFlags() & ~Qt::WindowCloseButtonHint) | Qt::CustomizeWindowHint);
#endif
#endif
}

void QxDialog::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Return)
		accept();
	else if (event->key() == Qt::Key_Escape)
		reject();
	else
		event->ignore();
}

} // namespace pacu
