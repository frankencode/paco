#include <QFormLayout>
#include "QxMiscPreferences.hpp"

namespace pacu
{

QxMiscPreferences::QxMiscPreferences(QWidget* parent)
{
	QFormLayout* form = new QFormLayout;
	#ifdef Q_WS_X11
	// workaround HACK
	form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	#endif
	
	kinematicScrolling_ = new QCheckBox(tr("Kinematic scrolling"), this);
	scrollingSpeed_ = new QSpinBox(this);
	form->addRow(tr("Scrolling Speed"), scrollingSpeed_);
	form->addRow(tr(" "), kinematicScrolling_);
	
	setLayout(form);
}

} // namespace pacu
