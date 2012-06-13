#include <QStackedLayout>
#include "QxWidgetStack.hpp"

namespace pacu
{

QxWidgetStack::QxWidgetStack(QWidget* parent)
	: QxControl(parent)
{
	layout_ = new QStackedLayout;
	layout_->setSpacing(0);
	layout_->setMargin(0);
	voidControl_ = new QxControl(this);
	layout_->addWidget(voidControl_);
	setLayout(layout_);
}

QxControl* QxWidgetStack::voidControl() const { return voidControl_; }

void QxWidgetStack::addWidget(QWidget* widget)
{
	layout_->addWidget(widget);
}

void QxWidgetStack::removeWidget(QWidget* widget)
{
	layout_->removeWidget(widget);
}

void QxWidgetStack::activateWidget(QWidget* widget)
{
	layout_->setCurrentWidget(widget);
	setFocusProxy(widget);
}

} // namespace pacu
