#ifndef PACU_QXWIDGETSTACK_HPP
#define PACU_QXWIDGETSTACK_HPP

#include <QPointer>
#include "UseVide.hpp"

class QStackedLayout;

namespace pacu
{

class QxWidgetStack: public QxControl
{
	Q_OBJECT
	
public:
	QxWidgetStack(QWidget* parent = 0);
	
	QxControl* voidControl() const;
	
public slots:
	void addWidget(QWidget* widget);
	void removeWidget(QWidget* widget);
	void activateWidget(QWidget* widget);
	
private:
	QPointer<QxControl> voidControl_;
	QPointer<QStackedLayout> layout_;
};

} // namespace pacu

#endif // PACU_QXWIDGETSTACK_HPP
