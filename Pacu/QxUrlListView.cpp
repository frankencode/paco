#include <QFocusEvent>
#include "QxUrlListView.hpp"

namespace pacu
{

QxUrlListView::QxUrlListView(QWidget* parent)
	: QListView(parent)
{
	setSelectionMode(QAbstractItemView::SingleSelection);
	setAcceptDrops(true);
	setIconSize(QSize(16, 16));
}

/*QSize QxUrlListView::minimumSizeHint() const
{
	QSize sz = QSize(1, visualRect(model()->index(model()->rowCount() - 1, 0)).bottom() + 1);
	int h = QListView::minimumSizeHint().height();
	if (sz.height() < h) sz.setHeight(h);
	return sz;
}
*/
QSize QxUrlListView::sizeHint() const {
	QSize sz = QSize(1, visualRect(model()->index(model()->rowCount() - 1, 0)).bottom() + 1);
	int h = QListView::minimumSizeHint().height();
	if (sz.height() < h) sz.setHeight(h);
	return sz;
}

void QxUrlListView::focusOutEvent(QFocusEvent* event)
{
	if (event->reason() != Qt::PopupFocusReason) {
		selectionModel()->clear();
		setCurrentIndex(QModelIndex());
	}
}

} // namespace pacu
