#include <QFormLayout>
#include <QButtonGroup>
#include <QDebug>
#include "QxEditorPreferences.hpp"

namespace pacu
{

QxEditorPreferences::QxEditorPreferences(QWidget* parent)
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
		subpixelAntialiasing_ = new QCheckBox(tr("Subpixel antialiasing (if supported)"), this);
		showWhitespace_ = new QCheckBox(tr("Show whitespace"), this);
		showLineNumbers_ = new QCheckBox(tr("Show line numbers"), this);
		
		form->addRow(tr("Render Options"), fontAntialiasing_);
		form->addRow(" ", subpixelAntialiasing_);
		form->addRow(" ", showWhitespace_);
		form->addRow(" ", showLineNumbers_);
	}
	{
		autoIndent_ = new QCheckBox(tr("Automatically indent new lines"), this);
		tabIndentMode_ = new QRadioButton(tr("Use tabulator characters (Code 9)"), this);
		spaceIndentMode_ = new QRadioButton(tr("Use space characters (Code 32)"), this);
		QButtonGroup* group = new QButtonGroup(this);
		group->addButton(tabIndentMode_);
		group->addButton(spaceIndentMode_);
		
		form->addRow(tr("Indent Mode"), autoIndent_);
		form->addRow(" ", tabIndentMode_);
		form->addRow(" ", spaceIndentMode_);
	}
	{
		tabWidth_ = new QSpinBox(this);
		form->addRow(tr("Tabulator Width"), tabWidth_);
	}
	{
		indentWidth_ = new QSpinBox(this);
		form->addRow(tr("Indent Width"), indentWidth_);
	}
	setLayout(form);
}

} // namespace pacu
