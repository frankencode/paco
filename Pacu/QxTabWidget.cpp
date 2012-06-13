#include <QVBoxLayout>
#include "QxTabBar.hpp"
#include "QxWidgetStack.hpp"
#include "QxTabWidget.hpp"

namespace pacu
{

QxTabWidget::QxTabWidget(QWidget* parent)
	: QxControl(parent)
{
	tabBar_ = new QxTabBar(this);
	widgetStack_ = new QxWidgetStack(this);
	widgetStack_->voidControl()->setVisual(new QxVisual(styleManager()->style("tabWidgetVoid")));
	connect(tabBar_, SIGNAL(addWidget(QWidget*)), widgetStack_, SLOT(addWidget(QWidget*)));
	connect(tabBar_, SIGNAL(removeWidget(QWidget*)), widgetStack_, SLOT(removeWidget(QWidget*)));
	connect(
		tabBar_, SIGNAL(changeActiveWidget(QWidget*, QWidget*)),
		this, SLOT(changeActiveWidget(QWidget*, QWidget*))
	);
	
	widgetStack_->setAcceptDrops(true);
	widgetStack_->installEventFilter(tabBar_);
	
	QVBoxLayout* col = new QVBoxLayout;
	col->setSpacing(0);
	col->setMargin(0);
	col->addWidget(tabBar_);
	col->addWidget(widgetStack_);
	setLayout(col);
	
	setFocusProxy(widgetStack_);
}

QxTabBar* QxTabWidget::tabBar() const { return tabBar_; }
QxWidgetStack* QxTabWidget::widgetStack() const { return widgetStack_; }

void QxTabWidget::changeActiveWidget(QWidget* old, QWidget* now)
{
	widgetStack_->activateWidget(now);
}

} // namespace pacu
