#include <QTimer>
#include <QWheelEvent>
#include "ScrollMachine.hpp"

namespace pte
{

ScrollMachine::ScrollMachine(QObject* parent)
	: QObject(parent),
	  drag_(0.05),
	  trans_(0.5),
	  tout_(0.1),
	  voff_(0.88),
	  speed_(0)
{}

void ScrollMachine::wheel(QWheelEvent* event)
{
	int steps = event->delta() / (8 * 15);
	if (steps == 0) steps = (event->delta() > 0) - (event->delta() < 0);
	wheel(steps);
}

void ScrollMachine::wheel(int steps)
{
	speed_ += steps * trans_;
	if (int(speed_) == 0) speed_ = steps;
	drive();
}

void ScrollMachine::drive()
{
	int steps = int(speed_);
	if ((speed_ < -voff_) || (voff_ < speed_))
		QTimer::singleShot(tout_ * 1000, this, SLOT(drive()));
	else
		speed_ = 0;
	speed_ *= 1. - drag_;
	if (steps != 0)
		emit scroll(steps);
}

} // namespace pte
