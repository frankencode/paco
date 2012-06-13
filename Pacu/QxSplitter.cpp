#include "QxApplication.hpp"
#include "QxSplitterHandle.hpp"
#include "QxSplitter.hpp"

namespace pacu
{

QxSplitter::QxSplitter(QWidget* parent)
	: QSplitter(parent)
{
	// performance HACK
	setAttribute(Qt::WA_NoSystemBackground, true);
	
	setHandleWidth(1);
	setChildrenCollapsible(false);
}

QxSplitter::QxSplitter(Qt::Orientation orientation, QWidget* parent)
	: QSplitter(orientation, parent)
{
	setHandleWidth(1);
	setChildrenCollapsible(false);
}

void QxSplitter::setHandle(int index, QWidget* handle) { handles_[index] = handle; }

QSplitterHandle* QxSplitter::createHandle()
{
	QxSplitterHandle* handle = 0;
	
	if (handles_.contains(count()))
		handle = new QxSplitterHandle(orientation(), this, count(), handles_[count()]);
	else
		handle = new QxSplitterHandle(orientation(), this, count());
	
	return handle;
}

} // namespace pacu
