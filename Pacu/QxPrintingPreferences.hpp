#ifndef PACU_QXPRINTINGPREFERENCES_HPP
#define PACU_QXPRINTINGPREFERENCES_HPP

#include <QWidget>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>

namespace pacu
{

class QxPrintingPreferences: public QWidget
{
public:
	QxPrintingPreferences(QWidget* parent = 0);
	
private:
	friend class QxPreferences;
	
	QFontComboBox* font_;
	QSpinBox* fontSize_;
	QCheckBox* fontAntialiasing_;
	QSpinBox* lineSpacing_;
	QCheckBox* showWhitespace_;
	QCheckBox* showLineNumbers_;
	QCheckBox* pageHeader_;
	QCheckBox* pageBorder_;
};

} // namespace pacu

#endif // PACU_QXPRINTINGPREFERENCES_HPP
