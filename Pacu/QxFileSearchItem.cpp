#include <QDebug>
#include <QTimer>
#include <QLayout>
#include "QxFileSearchItem.hpp"

namespace pacu
{

QxFileSearchItem::QxFileSearchItem(QWidget* parent, QPixmap icon, QString label, QString details)
	: QxControl(parent, new QxVisual(styleManager()->style("searchEditItem"))),
	  label_(label),
	  details_(details),
	  ignoreFirstTime_(false)
{
	setMode(QxControl::TouchMode);
	QTimer::singleShot(0, this, SLOT(activate()));
	connect(this, SIGNAL(selected()), this, SLOT(forward()));
	connect(this, SIGNAL(pressed()), this, SLOT(submit()));
	
	QGridLayout* grid = new QGridLayout;
	grid->setSpacing(0);
	grid->setMargin(0);
	{
		QxControl* iconCarrier = new QxControl(this);
		iconCarrier->setInheritState(true);
		iconCarrier->visual()->setLeadingIcon(icon);
		grid->addWidget(iconCarrier, 0, 0);
	}
	{
		QxControl* labelCarrier = new QxControl(this, new QxVisual(styleManager()->style("searchEditItemLabel")));
		labelCarrier->setInheritState(true);
		labelCarrier->visual()->setText(label);
		grid->addWidget(labelCarrier, 0, 1);
	}
	{
		QxControl* detailsCarrier = new QxControl(this, new QxVisual(styleManager()->style("searchEditItemDetails")));
		detailsCarrier->setInheritState(true);
		detailsCarrier->visual()->setText(details);
		grid->addWidget(detailsCarrier, 1, 1);
	}
	grid->addWidget(styleManager()->space(-1, -1, this), 0, 2);
	setLayout(grid);
}

QString QxFileSearchItem::details() const { return details_; }

void QxFileSearchItem::activate() {
	ignoreFirstTime_ = QRect(0, 0, width(), height()).contains(mapFromGlobal(QCursor::pos()));
	connect(this, SIGNAL(entered()), this, SLOT(select()));
}

void QxFileSearchItem::forward() {
	if (!ignoreFirstTime_)
		emit selected(label_);
}

void QxFileSearchItem::submit() {
	select();
	emit submitted(details_);
}

} // namespace pacu
