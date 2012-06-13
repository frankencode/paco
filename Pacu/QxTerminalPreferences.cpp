#include <QFormLayout>
#include "QxTerminalPreferences.hpp"

namespace pacu
{

QxTerminalPreferences::QxTerminalPreferences(QWidget* parent)
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
		form->addRow(tr("Render Options"), fontAntialiasing_);
	}
	{
		subpixelAntialiasing_ = new QCheckBox(tr("Subpixel antialiasing (if supported)"), this);
		form->addRow(" ", subpixelAntialiasing_);
	}
	{
		numberOfLines_ = new QSpinBox(this);
		form->addRow(tr("Number of Lines"), numberOfLines_);
	}
	{
		endlessLogging_ = new QCheckBox(tr("Endless logging"), this);
		form->addRow(" ", endlessLogging_);
	}
	{
		palette_ = new QComboBox(this);
		form->addRow(tr("Color Palette"), palette_);
	}
	{
		title_ = new QComboBox(this);
		title_->setEditable(true);
		form->addRow(tr("Title"), title_);
	}
	setLayout(form);
}

} // namespace pacu
