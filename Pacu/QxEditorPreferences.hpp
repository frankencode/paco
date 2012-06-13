#ifndef PACU_QXEDITORPREFERENCES_HPP
#define PACU_QXEDITORPREFERENCES_HPP

#include <QWidget>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>

namespace pacu
{

class QxPreferences;

class QxEditorPreferences: public QWidget
{
public:
	QxEditorPreferences(QWidget* parent = 0);
	
private:
	friend class QxPreferences;
	
	QFontComboBox* font_;
	QSpinBox* fontSize_;
	QCheckBox* fontAntialiasing_;
	QCheckBox* subpixelAntialiasing_;
	QSpinBox* lineSpacing_;
	QCheckBox* showWhitespace_;
	QCheckBox* showLineNumbers_;
	QCheckBox* autoIndent_;
	QRadioButton* tabIndentMode_;
	QRadioButton* spaceIndentMode_;
	QSpinBox* tabWidth_;
	QSpinBox* indentWidth_;
};

} // namespace pacu

#endif // PACU_QXEDITORPREFERENCES_HPP
