#ifndef PACU_QXTERMINALPREFERENCES_HPP
#define PACU_QXTERMINALPREFERENCES_HPP

#include <QWidget>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>

namespace pacu
{

class QxPreferences;

class QxTerminalPreferences: public QWidget
{
public:
	QxTerminalPreferences(QWidget* parent = 0);
	
private:
	friend class QxPreferences;
	
	QFontComboBox* font_;
	QSpinBox* fontSize_;
	QCheckBox* fontAntialiasing_;
	QCheckBox* subpixelAntialiasing_;
	QSpinBox* lineSpacing_;
	QCheckBox* endlessLogging_;
	QSpinBox* numberOfLines_;
	QComboBox* palette_;
	QComboBox* title_;
};

} // namespace pacu

#endif // PACU_QXTERMINALPREFERENCES_HPP
