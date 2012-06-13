#include <QDebug>
#include <QFrame>
#include <QMouseEvent>
#include <QApplication>
#include "QxListPopup.hpp"

namespace pacu
{

QxListPopup::QxListPopup(QxControl* combo, int align)
	: combo_(combo),
	  align_(align),
	  itemList_(new ItemList),
	  itemSeparators_(new ItemList),
	  maxItemWidth_(0)
{
	setWindowFlags(Qt::ToolTip);
	setAttribute(Qt::WA_ShowWithoutActivating);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setFocusPolicy(Qt::NoFocus);
	
	itemLayout_ = new QVBoxLayout;
	itemLayout_->setSpacing(0);
	itemLayout_->setMargin(0);
	#ifdef Q_WS_X11
	{
		QGridLayout* grid = new QGridLayout;
		grid->setSpacing(0);
		grid->setMargin(0);
		grid->addWidget(hl(), 0, 0, 1, 2);
		grid->addWidget(vl(), 1, 0);
		grid->addWidget(vl(), 1, 2);
		grid->addLayout(itemLayout_, 1, 1);
		setLayout(grid);
	}
	#endif
	#ifdef Q_WS_MAC
	setLayout(itemLayout_);
	#endif
	
	itemList_->setObserver(this);
	
	qApp->installEventFilter(this);
	// qApp->installEventFilter(combo->window());
}

QxListPopup::ItemList* QxListPopup::itemList() const { return itemList_; }

void QxListPopup::show()
{
	if (itemList_->length() == 0) {
		if (isVisible()) hide();
		return;
	}
	if (isVisible()) return;
	
	layout()->activate();
	QSize sz = sizeHint();
	int w = sz.width(), h = sz.height();
	
	QWidget* parentWidget = combo_->parentWidget();
	if (!parentWidget) parentWidget = combo_;
	QPoint orign;
	if (align_ == Qt::AlignLeft)
		orign = parentWidget->mapToGlobal(combo_->geometry().bottomLeft());
	else
		orign = parentWidget->mapToGlobal(combo_->geometry().bottomRight() - QPoint(w, 0));
	orign -= QPoint(3, 3); // visual HACK
	
	for (int i = 0, n = itemSeparators_->length(); i < n; ++i) { // workaround HACK to not show the last separator
		bool visible = (i != n -1);
		Ref<QxControl> sep = itemSeparators_->get(i);
		if (sep->isVisible() != visible)
			sep->setVisible(visible);
	}
	
	setGeometry(orign.x(), orign.y(), w, h);
	
	QWidget::show();
}

void QxListPopup::unselectAll()
{
	for (int i = 0, n = itemList_->length(); i < n; ++i)
		itemList_->get(i)->unselect();
}

void QxListPopup::rotateSelection()
{
	if (!isVisible()) { show(); return; }
	int n = itemList_->length();
	if (n == 0) return;
	bool nonSelected = !itemList_->get(n - 1)->state(QxControl::Selected);
	for (int i = n - 1; i > 0; --i) {
		QxControl* item0 = itemList_->get(i - 1);
		QxControl* item1 = itemList_->get(i);
		if (item0->state(QxControl::Selected)) {
			nonSelected = false;
			item0->unselect();
			item1->select();
		}
	}
	if (nonSelected)
		itemList_->get(0)->select();
}

void QxListPopup::backRotateSelection()
{
	if (!isVisible()) show();
	int n = itemList_->length();
	if (n == 0) return;
	bool nonSelected = !itemList_->get(0)->state(QxControl::Selected);
	for (int i = 1; i < n; ++i) {
		QxControl* item0 = itemList_->get(i - 1);
		QxControl* item1 = itemList_->get(i);
		if (item1->state(QxControl::Selected)) {
			nonSelected = false;
			item0->select();
			item1->unselect();
		}
	}
	if (nonSelected)
		itemList_->get(n - 1)->select();
}

QxControl* QxListPopup::selectedItem() const
{
	QxControl* item = 0;
	for (int i = 0, n = itemList_->length(); i < n; ++i) {
		if (itemList_->get(i)->state(QxControl::Selected)) {
			item = itemList_->get(i);
			break;
		}
	}
	return item;
}

bool QxListPopup::eventFilter(QObject* watched, QEvent* event)
{
	if (isVisible()) {
		if (event->type() == QEvent::MouseButtonPress) {
			//QMouseEvent* me = static_cast<QMouseEvent*>(event);
			//if (!QRect(combo_->mapToGlobal(QPoint(0, 0)), combo_->size()).contains(me->globalPos())) {
				emit stopResponseTimer();
				hide();
			//}
		}
		else if (watched == combo_->window()) {
			if ((event->type() == QEvent::Move) || (event->type() == QEvent::Resize)) {
				hide();
			}
		}
	}
	return QWidget::eventFilter(watched, event);
}

QxControl* QxListPopup::hl() { return styleManager()->hl(this); }
QxControl* QxListPopup::vl() { return styleManager()->vl(this); }

void QxListPopup::afterPush(int i, int n)
{
	for (int j = i; j < i + n; ++j) {
		QxControl* item = itemList_->get(j);
		QxControl* sep = hl();
		item->show();
		sep->show();
		itemSeparators_->insert(j, sep);
		itemLayout_->insertWidget(2 * j, itemList_->get(j));
		itemLayout_->insertWidget(2 * j + 1, sep);
	}
}

bool QxListPopup::beforePop(int i, int n)
{
	for (int j = i; j < i + n; ++j) {
		QxControl* item = itemList_->get(j);
		QxControl* sep = itemSeparators_->get(j);
		item->hide();
		sep->hide();
	}
	
	for (int j = 0; j < n; ++j)
		itemSeparators_->remove(i);
	
	return true;
}

} // namespace pacu
