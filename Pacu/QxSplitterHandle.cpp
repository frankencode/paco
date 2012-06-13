#include <QLayout>
#include "UseVide.hpp"
#include "QxSplitterHandle.hpp"

namespace pacu
{

QxSplitterHandle::QxSplitterHandle(Qt::Orientation orientation, QSplitter* parent, int index, QWidget* bar)
	: QSplitterHandle(orientation, parent),
	  index_(index)
{
	QBoxLayout* cell0 = 0;
	if (orientation == Qt::Vertical)
		cell0 = new QVBoxLayout;
	else
		cell0 = new QHBoxLayout;
	
	if (!bar)
		bar = new QxControl(this, new QxVisual(styleManager()->style("border")));
	
	cell0->setSpacing(0);
	cell0->setMargin(0);
	if (parent->handleWidth() > 1) {
		QxControl* sep0 = new QxControl(this, new QxVisual(styleManager()->style("border")));
		QxControl* sep1 = new QxControl(this, new QxVisual(styleManager()->style("border")));
		if (orientation == Qt::Vertical) {
			sep0->setFixedHeight(1);
			sep1->setFixedHeight(1);
		}
		else {
			sep0->setFixedWidth(1);
			sep1->setFixedWidth(1);
		}
		cell0->addWidget(sep0);
		cell0->addWidget(bar);
		cell0->addWidget(sep1);
	}
	else {
		cell0->addWidget(bar);
	}
	setLayout(cell0);
}

bool QxSplitterHandle::collapsed() const
{
	if (index_ == 0) return false;
	return splitter()->widget(index_-1)->rect().isEmpty() ||
	       splitter()->widget(index_)->rect().isEmpty();
}

} // namespace pacu
