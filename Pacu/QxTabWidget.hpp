#ifndef PACU_QXTABWIDGET_HPP
#define PACU_QXTABWIDGET_HPP

#include "UseVide.hpp"

namespace pacu
{

class QxTabBar;
class QxWidgetStack;

class QxTabWidget: public QxControl
{
	Q_OBJECT
	
public:
	QxTabWidget(QWidget* parent = 0);
	
	QxTabBar* tabBar() const;
	QxWidgetStack* widgetStack() const;
	
private slots:
	void changeActiveWidget(QWidget* old, QWidget* now);
	
private:
	QPointer<QxTabBar> tabBar_;
	QPointer<QxWidgetStack> widgetStack_;
};

} // namespace pacu

#endif // PACU_QXTABWIDGET_HPP
