#ifndef PACU_QXTESTPALETTE_HPP
#define PACU_QXTESTPALETTE_HPP

#include <QtGui>

namespace pacu
{

class QxTestPalette: public QWidget
{
public:
	QxTestPalette(QPalette::ColorGroup colorGroup = QPalette::Active, QWidget* parent = 0);
	
private:
	void paintEvent(QPaintEvent* event);
	
	QPalette::ColorGroup colorGroup_;
};

} // namespace pacu

#endif // PACU_QXTESTPALETTE_HPP

