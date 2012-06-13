#include <QLayout>
#include <vide/QxStyleManager.hpp>
#include "QxStatusBar.hpp"

namespace pacu
{

QxStatusBar::QxStatusBar(QWidget* parent)
	: QWidget(parent)
{
	QxStyle* finish = styleManager()->style("finish");
	
	left_ = new QxControl(this, new QxVisual(new QxStyle(finish)));
	center_ = new QxControl(this, new QxVisual(new QxStyle(finish)));
	right_ = new QxControl(this, new QxVisual(new QxStyle(finish)));
	
	center_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	left_->visual()->style()->setElideMode(Qt::ElideRight);
	
	int n = QFontMetrics(finish->font()).width('n');
	left_->visual()->style()->setMargins(0, n, 0, n);
	center_->visual()->style()->setMargins(0, n, 0, n);
	right_->visual()->style()->setMargins(0, n, 0, n);
	
	QHBoxLayout* cell0 = new QHBoxLayout;
	cell0->setMargin(0);
	cell0->setSpacing(0);
	cell0->addWidget(left_);
	cell0->addWidget(center_);
	cell0->addWidget(right_);
	setLayout(cell0);
}

void QxStatusBar::display(int place, QString text)
{
	if (place == Left)
		left_->visual()->setText(text);
	else if (place == Center)
		center_->visual()->setText(text);
	else if (place == Right)
		right_->visual()->setText(text);
}

} // namespace pacu
