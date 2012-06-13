#ifndef PACU_QXSTATUSBAR_HPP
#define PACU_QXSTATUSBAR_HPP

#include "UseVide.hpp"

namespace pacu
{

class QxStatusBar: public QWidget
{
	Q_OBJECT
	
public:
	QxStatusBar(QWidget* parent = 0);
	
	enum Place { Left, Center, Right };
	
	void display(int place, QString text);
	
private:
	QxControl* left_;
	QxControl* center_;
	QxControl* right_;
};

} // namespace pacu

#endif // PACU_QXSTATUSBAR_HPP

