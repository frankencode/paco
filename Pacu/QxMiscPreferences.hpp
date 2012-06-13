#ifndef PACU_QXMISCPREFERENCES_HPP
#define PACU_QXMISCPREFERENCES_HPP

#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>

namespace pacu
{

class QxMiscPreferences: public QWidget
{
public:
	QxMiscPreferences(QWidget* parent = 0);
	
private:
	friend class QxPreferences;
	
	QCheckBox* kinematicScrolling_;
	QSpinBox* scrollingSpeed_;
	
};

} // namespace pacu

#endif // PACU_QXMISCPREFERENCES_HPP
