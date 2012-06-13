#include <QFormLayout>
#include "QxPrintingPreferences.hpp"

namespace pacu
{

QxPrintingPreferences::QxPrintingPreferences(QWidget* parent)
	: QWidget(parent)
{
	QFormLayout* form = new QFormLayout;
	#ifdef Q_WS_X11
	// workaround HACK
	form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	#endif
	{
		font_ = new QFontComboBox(this);
		form->addRow(tr("Font"), font_);
	}
	{
		fontSize_ = new QSpinBox(this);
		form->addRow(tr("Font Size"), fontSize_);
	}
	{
		lineSpacing_ = new QSpinBox(this);
		form->addRow(tr("Line Spacing"), lineSpacing_);
	}
	{
		fontAntialiasing_ = new QCheckBox(tr("Font antialiasing"), this);
		showWhitespace_ = new QCheckBox(tr("Show whitespace"), this);
		showLineNumbers_ = new QCheckBox(tr("Show line numbers"), this);
		
		form->addRow(tr("Render Options"), fontAntialiasing_);
		form->addRow(" ", showWhitespace_);
		form->addRow(" ", showLineNumbers_);
	}
	{
		pageHeader_ = new QCheckBox(tr("Embed page header"), this);
		pageBorder_ = new QCheckBox(tr("Mark page border"), this);
		
		form->addRow(tr("Page Style"), pageHeader_);
		form->addRow(" ", pageBorder_);
	}
	setLayout(form);
}

} // namespace pacu
