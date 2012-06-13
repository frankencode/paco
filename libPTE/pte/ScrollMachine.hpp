#ifndef PTE_SCROLLMACHINE_HPP
#define PTE_SCROLLMACHINE_HPP

#include <QObject>

class QWheelEvent;

namespace pte
{

class ScrollMachine: public QObject
{
	Q_OBJECT
	
public:
	ScrollMachine(QObject* parent = 0);
	
	void wheel(QWheelEvent* event);
	void wheel(int steps);
	
signals:
	void scroll(int steps);
	
private slots:
	void drive();
	
private:
	qreal drag_; // mechanical friction
	qreal trans_; // transmission
	qreal tout_; // auto-scroll timeout
	qreal voff_; // minimum glide speed
	qreal speed_; // current speed
};

} // namespace pte

#endif // PTE_SCROLLMACHINE_HPP
