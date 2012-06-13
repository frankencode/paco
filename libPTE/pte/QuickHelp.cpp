#include <QDebug> // DEBUG
#include <QLabel>
#include <QLayout>
#include <QDesktopServices>
#include <QUrl>
#include "QuickHelp.hpp"

namespace pte
{

QuickHelp::QuickHelp(QWidget* widget, QPoint pos, QString text)
	: QFrame(0, Qt::ToolTip),
	  widget_(widget),
	  window_(widget->window())
{
	setAttribute(Qt::WA_DeleteOnClose);
	
	widget_->installEventFilter(this);
	window_->installEventFilter(this);
	
	QLabel* label = new QLabel;
	label->setStyleSheet(
		QString("QLabel { background-color: %1; }").arg(label->palette().toolTipBase().color().name())
	);
	label->setTextFormat(Qt::RichText);
	// label->setOpenExternalLinks(true);
	label->setText(text);
	connect(label, SIGNAL(linkActivated(const QString&)), this, SLOT(openLink(const QString&)));
	
	#ifdef QT_MAC_USE_COCOA
	setFrameStyle(QFrame::NoFrame);
	#else
	setFrameStyle(QFrame::Plain|QFrame::StyledPanel);
	#endif
	
	QVBoxLayout* col = new QVBoxLayout;
	col->setSpacing(0);
	col->setMargin(0);
	col->addWidget(label);
	setLayout(col);
	
	move(widget->mapToGlobal(pos));
	show();
}

void QuickHelp::openLink(const QString& path)
{
	QDesktopServices::openUrl(path);
}

bool QuickHelp::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == widget_) {
		if ( (event->type() == QEvent::KeyPress) ||
		     (event->type() == QEvent::MouseButtonPress) ||
		     (event->type() == QEvent::Wheel) ||
		     (event->type() == QEvent::FocusOut) ||
		     (event->type() == QEvent::InputMethod) ) {
			close();
		}
	}
	else if (obj == window_) {
		if ((event->type() == QEvent::ApplicationActivate) ||
		    (event->type() == QEvent::ApplicationDeactivate) ||
		    (event->type() == QEvent::MouseButtonPress) ||
		    (event->type() == QEvent::Move) ||
		    (event->type() == QEvent::Resize) ||
		    (event->type() == QEvent::Shortcut) ||
		    (event->type() == QEvent::WindowActivate) ||
		    (event->type() == QEvent::WindowDeactivate)) {
			close();
		}
	}
	return false;
}

} // namespace pte
