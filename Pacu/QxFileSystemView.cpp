#include <QDebug> // DEBUG
#include "QxFileSystemView.hpp"

namespace pacu
{

QxFileSystemView::QxFileSystemView(QWidget* parent)
	: QTreeView(parent),
	  ignoreNextFocusOutEvent_(false)
{
	setEditTriggers(QAbstractItemView::SelectedClicked|QAbstractItemView::EditKeyPressed);
	setUniformRowHeights(true); // performance HACK
	setDragDropMode(QAbstractItemView::DragDrop);
	setAutoExpandDelay(700);
	setTextElideMode(Qt::ElideMiddle);
	// setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void QxFileSystemView::ignoreNextFocusOutEvent()
{
	ignoreNextFocusOutEvent_ = true;
}

void QxFileSystemView::resizeColumnsToContents()
{
	/*int numVisible = 0;
	for (int k = 0; k < model()->columnCount(); ++k)
		numVisible += !isColumnHidden(k);*/
	
	for (int k = 0; k < model()->columnCount(); ++k) {
		if (!isColumnHidden(k)) {
		/*	if (numVisible == 1) {
				QTreeView::setColumnWidth(k, width());
				break;
			}
			else*/
				QTreeView::resizeColumnToContents(k);
		}
	}
}

void QxFileSystemView::keyPressEvent(QKeyEvent* event)
{
	bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0;
	int key = event->key();
	if (key == Qt::Key_Delete)
		emit delKeyPressed();
	else if (key == Qt::Key_Space)
		emit activated(currentIndex());
	else if (key == Qt::Key_Escape)
		emit escape();
	else if (ctrl & (key == Qt::Key_Up))
		emit cdUp();
	else
		QTreeView::keyPressEvent(event);
}

/*void QxFileSystemView::resizeEvent(QResizeEvent* event)
{
	resizeColumnsToContents();
	QTreeView::resizeEvent(event);
}*/

/*void QxFileSystemView::focusInEvent(QFocusEvent* event)
{
	// qDebug() << "QxFileSystemView::focusInEvent(): event->reason() =" << event->reason();
	//if (event->reason() != Qt::OtherFocusReason) {
		if (currentIndexSaved_.isValid())
			setCurrentIndex(currentIndexSaved_);
		else
			setCurrentIndex(moveCursor(QAbstractItemView::MoveHome, 0));
	//}
	QTreeView::focusInEvent(event);
}

void QxFileSystemView::focusOutEvent(QFocusEvent* event)
{
	// qDebug() << "QxFileSystemView::focusOutEvent(): event->reason() =" << event->reason();
	//if (event->reason() != Qt::OtherFocusReason) {
		currentIndexSaved_ = currentIndex();
		//if (event->reason() != Qt::PopupFocusReason)
		//	setCurrentIndex(QModelIndex());
	//}
	QTreeView::focusOutEvent(event);
}*/

void QxFileSystemView::contextMenuEvent(QContextMenuEvent* event)
{
	event->ignore(); // propagate to parent widget
}

#ifdef QT_MAC_USE_COCOA
void QxFileSystemView::mouseMoveEvent(QMouseEvent* event)
{
	if (!hasFocus())
		event->ignore();
	else
		QTreeView::mouseMoveEvent(event);
}
#endif
/*void QxFileSystemView::rowsInserted(const QModelIndex & parent, int start, int end)
{
	QFileSystemModel* model = qobject_cast<QFileSystemModel*>(this->model());
	qDebug() << "rowsInserted(" << start << "," << end << ")";
	for (int y = start; y <= end; ++y)
		qDebug() << model->data(model->index(y, 0));
	QTreeView::rowsInserted(parent, start, end);
}*/

/*void QxFileSystemView::keyboardSearch(const QString& search)
{
	qDebug() << "search =" << search;
	QAbstractItemView::keyboardSearch(search);
}*/

/*void QxFileSystemView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	qDebug() << "QxFileSystemView::dataChanged(): topLeft, bottomRight =" << topLeft << "," << bottomRight;
}*/

} // namespace pacu
