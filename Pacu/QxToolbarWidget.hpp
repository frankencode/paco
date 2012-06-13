#ifndef PACU_QXTOOLBARWIDGET_HPP
#define PACU_QXTOOLBARWIDGET_HPP

#include <QWidgetAction>

namespace pacu
{

class QxToolBarWidget: public QWidgetAction
{
	Q_OBJECT
	
public:
	QxToolBarWidget(QWidget* widget);
	
private:
	virtual QWidget* createWidget(QWidget* parent);
	virtual void deleteWidget(QWidget* widget);
	
	QWidget* widget_;
};

} // namespace pacu

#endif // PACU_QXTOOLBARWIDGET_HPP
