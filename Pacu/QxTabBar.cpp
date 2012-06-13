#include <QMouseEvent>
#include <QApplication>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QStyleOptionTab>
#include <ftl/Process.hpp>
#include "UseFtl.hpp"
#include "QxTabBar.hpp"

namespace pacu
{

QxTabBar::QxTabBar(QWidget* parent)
	: QxControl(parent),
	  style_(styleManager()->style("tabWidgetBar")),
	  tabStyle_(new QxStyle(styleManager()->style("tabWidgetTab"))),
	  gap_(styleManager()->constant("tabWidgetGap")),
	  acceptUriDrops_(false),
	  indexActive_(-1),
	  layoutDirection_(Qt::LeftToRight),
	  indexAlpha_(0),
	  indexBeta_(0),
	  dragIndex_(-1),
	  dropIndex_(-1)
{
	setAcceptDrops(true);
	
	tabMinWidth_ = tabStyle_->minWidth();
	tabStyle_->setMinWidth(0);
	
	carrier_ = new QxControl(this, new QxVisual(styleManager()->style("tabWidgetCarrier")));
	
	closeButton_ = new QxControl(carrier_, new QxVisual(styleManager()->style("tabWidgetCloseButton")));
	closeButton_->setMode(QxControl::TouchMode);
	connect(closeButton_, SIGNAL(pressed()), this, SLOT(closeTab()));
	
	moreButton_ = new QxControl(carrier_, new QxVisual(styleManager()->style("tabWidgetMoreButton")));
	moreButton_->setMode(QxControl::TouchMode);
	moreButton_->setVisible(false);
	connect(moreButton_, SIGNAL(pressed()), this, SLOT(moreTab()));
	
	dropMark_ = new QxControl(carrier_, new QxVisual(styleManager()->style("tabWidgetDropMark")));
	dropMark_->setMode(QxControl::DisplayMode);
	dropMark_->setVisible(false);
	
	filler_ = new QxControl(carrier_, new QxVisual(styleManager()->style("tabWidgetFiller")));
	filler_->setMode(QxControl::DisplayMode);
	filler_->setVisible(false);
	
	scrollLeft_ = new QxControl(carrier_, new QxVisual(styleManager()->style("tabWidgetScrollLeftButton")));
	scrollLeft_->setMode(QxControl::TouchMode);
	scrollLeft_->setVisible(false);
	connect(scrollLeft_, SIGNAL(pressed()), this, SLOT(stepLeft()));
	
	scrollRight_ = new QxControl(carrier_, new QxVisual(styleManager()->style("tabWidgetScrollRightButton")));
	scrollRight_->setMode(QxControl::TouchMode);
	scrollRight_->setVisible(false);
	connect(scrollRight_, SIGNAL(pressed()), this, SLOT(stepRight()));
	
	updateStyleHints();
}

void QxTabBar::openTab(int index, QString label, QPixmap icon, QWidget* widget)
{
	QxControl* tab = new QxControl(carrier_, new QxVisual(new QxStyle(tabStyle_)));
	tab->setMode(QxControl::RadioMode);
	tab->visual()->setText(label);
	if (!icon.isNull())
		tab->visual()->setLeadingIcon(icon);
	connect(tab, SIGNAL(contextMenuRequest(QPoint)), this, SLOT(showContextMenu(QPoint)));
	
	tabs_.insert(index, tab);
	widgets_.insert(index, widget);
	emit addWidget(widget);
	
	if (indexActive_ >= index)
		++indexActive_;
	
	layoutWidgets();
	activate(index);
}

void QxTabBar::closeTab(int index, bool closeWidget)
{
	if ((index < 0) || (tabs_.count() <= index))
		return;
	
	if (index == indexActive_) {
		if (index + 1 < tabs_.count())
			activate(index + 1);
		else if (index > 0)
			activate(index - 1);
		else
			indexActive_ = -1;
	}
	
	QxControl* tab = tabs_.at(index);
	QWidget* widget = widgets_.at(index);
	
	tabs_.removeAt(index);
	widgets_.removeAt(index);
	emit removeWidget(widget);
	
	if (index < indexActive_)
		--indexActive_;
	
	tab->close();
	
	if (closeWidget)
		if (widget)
			widget->close();
	
	layoutWidgets();
	update();
	
	if (tabs_.count() == 0)
		emit lastTabClosed();
}

int QxTabBar::tabCount() const { return tabs_.count(); }

void QxTabBar::activate(int index)
{
	if ((index < 0) || (tabs_.count() <= index))
		return;
	
	if (index == indexActive_) {
		if (!widgets_.at(index)->hasFocus())
			widgets_.at(index)->setFocus();
		return;
	}
	
	QWidget* old = 0;
	if (indexActive_ != -1) {
		old = widgets_.at(indexActive_);
		tabs_.at(indexActive_)->toggle(false);
	}
	
	indexActive_ = index;
	
	tabs_.at(index)->toggle(true);
	QWidget* now = widgets_.at(index);
	
	emit changeActiveWidget(old, now);
	if (now) now->setFocus();
	
	if ((indexAlpha_ == 0) && (indexBeta_ == 0))
		layoutWidgets();
	keepInView();
	layoutWidgets();
	update();
}

int QxTabBar::indexActive() const { return indexActive_; }

QString QxTabBar::label(int index) const
{
	if ((index < 0) || (tabs_.count() <= index))
		return QString();
	return tabs_.at(index)->visual()->text();
}

void QxTabBar::setLabel(int index, QString text)
{
	if ((index < 0) || (tabs_.count() <= index))
		return;
	tabs_.at(index)->visual()->setText(text);
	layoutWidgets(); // architecture HACK, we should process layout request event instead
	update();
}

QPixmap QxTabBar::icon(int index) const
{
	if ((index < 0) || (tabs_.count() <= index))
		return QPixmap();
	return tabs_.at(index)->visual()->leadingIcon();
}

void QxTabBar::setIcon(int index, QPixmap icon)
{
	if ((index < 0) || (tabs_.count() <= index))
		return;
	tabs_.at(index)->visual()->setLeadingIcon(icon);
}

QWidget* QxTabBar::widget(int index) const
{
	if ((index < 0) || (widgets_.count() <= index))
		return 0;
	return widgets_.at(index);
}

void QxTabBar::setWidget(int index, QWidget* widget)
{
	if ((index < 0) || (tabs_.count() <= index))
		return;
	widgets_[index] = widget;
	layoutWidgets();
	update();
}

QString QxTabBar::semanticType() const { return semanticType_; }
void QxTabBar::setSemanticType(QString name) { semanticType_ = name; }

bool QxTabBar::acceptUriDrops() const { return acceptUriDrops_; }
void QxTabBar::setAcceptUriDrops(bool on) { acceptUriDrops_ = on; }

QMenu* QxTabBar::contextMenu() const { return contextMenu_; }
void QxTabBar::setContextMenu(QMenu* menu) { contextMenu_ = menu; }

QSize QxTabBar::minimumSizeHint() const
{
	return QSize(128 /* save bet */, style_->minHeight());
}

QSize QxTabBar::sizeHint() const
{
	return minimumSizeHint();
}

void QxTabBar::dragTabLeft() { dragTab(-1); }
void QxTabBar::dragTabRight() { dragTab(1); }

void QxTabBar::dragTab(int distance)
{
	if ((tabs_.length() < 2) || (indexActive_ == -1)) return;
	
	if (dragIndex_ == -1) {
		dragIndex_ = indexActive_;
		dropIndex_ = indexActive_ + (distance > 0);
	}
	
	dropIndex_ += distance;
	
	if (dropIndex_ < 0)
		dropIndex_ = 0;
	else if (dropIndex_ > tabs_.length())
		dropIndex_ = tabs_.length();
	
	if (keepInView(dropIndex_ - (dropIndex_ == tabs_.length()))) {
		layoutWidgets();
		update();
	}
	
	int dmp = styleManager()->constant("tabWidgetDropMarkDisplacement");
	QSize h = dropMark_->sizeHint();
	if (dropIndex_ < tabs_.length()) {
		QxControl* tab = tabs_.at(dropIndex_);
		dropMark_->setGeometry(tab->x() - h.width() / 2 - gap_ - dmp, style_->minHeight() - h.height(), h.width(), h.height());
	}
	else {
		QxControl* tab = tabs_.at(tabs_.length() - 1);
		dropMark_->setGeometry(tab->x() + tab->width() - h.width() / 2 - dmp, style_->minHeight() - h.height(), h.width(), h.height());
	}
	
	if (!dropMark_->isVisible()) {
		dropMark_->setVisible(true);
		dropMark_->raise();
	}
}

void QxTabBar::dropTab()
{
	dropMark_->setVisible(false);
	moveTab(dragIndex_, dropIndex_);
	dragIndex_ = -1;
	dropIndex_ = -1;
}

void QxTabBar::closeTab()
{
	if (indexActive_ != -1)
		emit closeRequest(indexActive_);
}

void QxTabBar::moreTab()
{
	QMenu* menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose, true);
	// QxDesignHack::beautify(menu);
	actionToIndex_.clear();
	for (int i = 0; i < tabs_.count(); ++i) {
		QxControl* tab = tabs_.at(i);
		if ((!tab->isVisible()) || (tab->visual()->sizeHint(tabStyle_).width() != tab->width())) {
			QAction* action = menu->addAction(label(i));
			if (i == indexActive_) {
				action->setCheckable(true);
				action->setChecked(true);
			}
			actionToIndex_[action] = i;
		}
	}
	QSize sz = menu->sizeHint();
	QPoint orign = mapToGlobal(QPoint(width() - sz.width(), style_->minHeight()));
	menu->move(orign);
	menu->show();
	connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(activate(QAction*)));
}

void QxTabBar::activate(QAction* action)
{
	activate(actionToIndex_[action]);
}

void QxTabBar::stepLeft()
{
	bool squeezedTab = false;
	if (layoutDirection_ == Qt::RightToLeft)
		if (indexAlpha_ < tabs_.count())
			if (tabs_[indexAlpha_]->width() + 10  < tabs_[indexAlpha_]->visual()->sizeHint(tabStyle_).width())
				squeezedTab = true;
	layoutDirection_ = Qt::LeftToRight;
	if ((indexAlpha_ > 0) && (!squeezedTab))
		--indexAlpha_;
	layoutWidgets();
	update();
}

void QxTabBar::stepRight()
{
	bool squeezedTab = false;
	if (layoutDirection_ == Qt::LeftToRight)
		if (indexBeta_ < tabs_.count())
			if (tabs_[indexBeta_]->width() + 10 < tabs_[indexBeta_]->visual()->sizeHint(tabStyle_).width())
				squeezedTab = true;
	layoutDirection_ = Qt::RightToLeft;
	if ((indexBeta_ < tabs_.count() - 1) && (!squeezedTab))
		++indexBeta_;
	layoutWidgets();
	update();
}

void QxTabBar::showContextMenu(QPoint pos)
{
	if (contextMenu_) {
		QxControl* tab = qobject_cast<QxControl*>(sender());
		int index = -1;
		for (int i = 0, n = tabs_.count(); i < n; ++i) {
			if (tabs_.at(i) == tab) {
				index = i;
				break;
			}
		}
		activate(index);
		contextMenu_->exec(mapToGlobal(tab->geometry().bottomLeft() + QPoint(0, 1)));
	}
}

QString QxTabBar::internalMimeFormat() const
{
	return QString("pacu-binary/tab-index-%1").arg(Process::currentId());
}

bool QxTabBar::keepInView() { return keepInView(indexActive_); }

bool QxTabBar::keepInView(int tabIndex)
{
	bool needsRelayout = false;
	
	if ((tabIndex < 0) && (tabs_.count() <= tabIndex))
		return needsRelayout;
	
	QxControl* tab = tabs_.at(tabIndex);
	QSize h = tab->visual()->sizeHint(tabStyle_);
	
	if ((!tab->isVisible()) || (tab->width() != h.width())) {
		if (tabIndex == indexAlpha_) {
			layoutDirection_ = Qt::LeftToRight;
		}
		else if (tabIndex == indexBeta_) {
			layoutDirection_ = Qt::RightToLeft;
		}
		else if (tabIndex < indexAlpha_) {
			indexAlpha_ = tabIndex;
			layoutDirection_ = Qt::LeftToRight;
		}
		else if (indexBeta_ < tabIndex) {
			indexBeta_ = tabIndex;
			layoutDirection_ = Qt::RightToLeft;
		}
		needsRelayout = true;
	}
	
	return needsRelayout;
}

void QxTabBar::moveTab(int dragIndex, int dropIndex)
{
	if ((dragIndex == dropIndex) || (dragIndex + 1 == dropIndex))
		return;
	
	QxControl* tab = tabs_.at(dragIndex);
	QWidget* widget = widgets_.at(dragIndex);
	
	if (dragIndex < dropIndex)
		--dropIndex;
	tabs_.removeAt(dragIndex);
	widgets_.removeAt(dragIndex);
	tabs_.insert(dropIndex, tab);
	widgets_.insert(dropIndex, widget);
	
	indexActive_ = dropIndex; // a little unclean, design HACK
	
	layoutWidgets();
	if (keepInView())
		layoutWidgets();
}

void QxTabBar::updateStyleHints()
{
	for (int i = 0, n = tabs_.count(); i < n; ++i)
	{
		QxControl* tab = tabs_.at(i);
		if (i + 1 == indexActive_)
			tab->setProperty("selectedPosition", QStyleOptionTab::NextIsSelected);
		else if (i - 1 == indexActive_)
			tab->setProperty("selectedPosition", QStyleOptionTab::PreviousIsSelected);
		else
			tab->setProperty("selectedPosition", QStyleOptionTab::NotAdjacent);
		if (i == 0) {
			if (n == 1)
				tab->setProperty("position", QStyleOptionTab::OnlyOneTab);
			else
				tab->setProperty("position", QStyleOptionTab::Beginning);
		}
		else if (i == n -1)
			tab->setProperty("position", QStyleOptionTab::End);
		else
			tab->setProperty("position", QStyleOptionTab::Middle);
	}
	
	if ((indexActive_ == indexAlpha_) && (layoutDirection_ == Qt::LeftToRight))
		scrollLeft_->setProperty("selectedPosition", QStyleOptionTab::NextIsSelected);
	else
		scrollLeft_->setProperty("selectedPosition", QStyleOptionTab::NotAdjacent);
	
	if ((indexActive_ == indexAlpha_) && (layoutDirection_ == Qt::RightToLeft))
		scrollRight_->setProperty("selectedPosition", QStyleOptionTab::PreviousIsSelected);
	else
		scrollRight_->setProperty("selectedPosition", QStyleOptionTab::NotAdjacent);
	
	scrollLeft_->enable((indexAlpha_ > 0) || (layoutDirection_ == Qt::RightToLeft));
	scrollRight_->enable((indexBeta_ < tabs_.count() - 1) || (layoutDirection_ == Qt::LeftToRight));
	
	moreButton_->setProperty("selectedPosition", QStyleOptionTab::NotAdjacent);
	closeButton_->setProperty("selectedPosition", QStyleOptionTab::NextIsSelected);
	
	#ifdef Q_WS_MAC
	carrier_->setProperty("tabWidgetCarrierLeadingHighlight", true);
	#endif
}

void QxTabBar::layoutWidgets()
{
	updateStyleHints();
	setUpdatesEnabled(false);
	
	// compute overall width needed to layout all tabs
	bool overlong = false;
	{
		int widthTotal = 0;
		for (int i = 0; i < tabs_.count(); ++i) {
			QxControl* tab = tabs_.at(i);
			QSize h = tab->visual()->sizeHint(tabStyle_);
			if (h.width() < tabMinWidth_) h.setWidth(tabMinWidth_);
			widthTotal += h.width() + gap_;
		}
		
		widthTotal += closeButton_->sizeHint().width();
		
		overlong = (widthTotal > width());
	}
	
	// forbit any scrolling, if enough space is available
	if (!overlong) {
		layoutDirection_ = Qt::LeftToRight;
		indexAlpha_ = 0;
	}
	
	// limit to valid range (e.g. when tabs are closed)
	if (indexAlpha_ >= tabs_.count())
		indexAlpha_ = tabs_.count() - 1;
	if (indexBeta_ >= tabs_.count())
		indexBeta_ = tabs_.count() - 1;
	
	// define range to layout tabs
	int layoutStart = 0;
	int layoutStop = width() - closeButton_->sizeHint().width();
	if (overlong)
		layoutStop -= scrollRight_->sizeHint().width() + gap_ + moreButton_->sizeHint().width();
	
	// layout leading buttons
	if (overlong) {
		QSize h = scrollLeft_->sizeHint();
		scrollLeft_->setGeometry(layoutStart, 0, h.width(), h.height());
		layoutStart += h.width() + gap_;
		scrollLeft_->setVisible(true);
	}
	else {
		scrollLeft_->setVisible(false);
	}
	
	if (layoutDirection_ == Qt::LeftToRight)
	{
		// layout tabs from left to right
		
		indexBeta_ = tabs_.count() - 1;
		int layoutPos = layoutStart;
		
		for (int i = 0; i < tabs_.count(); ++i)
		{
			QxControl* tab = tabs_.at(i);
			if ((i < indexAlpha_) || (indexBeta_ < i)) {
				tab->setVisible(false);
			}
			else {
				tab->setVisible(true);
				tab->stackUnder(dropMark_); // workaround HACK
				
				QSize h = tab->visual()->sizeHint(tabStyle_);
				if (h.width() < tabMinWidth_) h.setWidth(tabMinWidth_);
				int spaceLeft = layoutStop - (layoutPos + h.width() + gap_);
				QxVisual* visual = tab->visual();
				if (spaceLeft < 0) {
					visual->style()->setElideMode(Qt::ElideRight);
					visual->style()->setTextAlign(Qt::AlignLeft|Qt::AlignVCenter);
					visual->style()->setTrailingPatch(QPixmap());
					// incomplete HACK, must also take care of possible new state HoverPatch
					h.setWidth(h.width() + spaceLeft);
					indexBeta_ = i;
				}
				else {
					visual->style()->setElideMode(tabStyle_->elideMode());
					visual->style()->setTextAlign(tabStyle_->textAlign());
					visual->style()->setTrailingPatch(tabStyle_->trailingPatch());
					// incomplete HACK
				}
				tab->setGeometry(layoutPos, 0, h.width(), h.height());
				layoutPos += h.width() + gap_;
			}
		}
		
		if (layoutPos + gap_ < layoutStop) {
			if (indexAlpha_ > 0) {
				layoutDirection_ = Qt::RightToLeft;
				layoutWidgets();
				return;
			}
			
			filler_->setGeometry(layoutPos, 0, layoutStop - layoutPos - 1, style_->minHeight());
			filler_->setVisible(true);
		}
		else {
			filler_->setVisible(false);
		}
	}
	else if (layoutDirection_ == Qt::RightToLeft)
	{
		// layout tabs from right to left
		
		indexAlpha_ = 0;
		int layoutPos = layoutStop;
		
		for (int i = tabs_.count() - 1; i >= 0; --i)
		{
			QxControl* tab = tabs_.at(i);
			if ((i < indexAlpha_) || (indexBeta_ < i)) {
				tab->setVisible(false);
			}
			else {
				tab->setVisible(true);
				tab->stackUnder(dropMark_); // workaround HACK
				
				QSize h = tab->visual()->sizeHint(tabStyle_);
				if (h.width() < tabMinWidth_) h.setWidth(tabMinWidth_);
				int spaceLeft = (layoutPos - h.width() - gap_) - layoutStart;
				QxVisual* visual = tab->visual();
				visual->style()->setTrailingPatch(tabStyle_->trailingPatch());
				// incomplete HACK
				if (spaceLeft < 0) {
					visual->style()->setElideMode(Qt::ElideLeft);
					visual->style()->setTextAlign(Qt::AlignRight|Qt::AlignVCenter);
					h.setWidth(h.width() + spaceLeft);
					indexAlpha_ = i;
				}
				else {
					visual->style()->setElideMode(tabStyle_->elideMode());
					visual->style()->setTextAlign(tabStyle_->textAlign());
				}
				layoutPos -= h.width() + gap_;
				tab->setGeometry(layoutPos, 0, h.width(), h.height());
			}
		}
		
		if (layoutPos > layoutStart) {
			layoutDirection_ = Qt::LeftToRight;
			layoutWidgets();
			return;
		}
		
		filler_->setVisible(false);
	}
	
	// layout trailing buttons
	{
		int layoutPos = layoutStop;
		
		if (overlong)
		{
			{
				QSize h = scrollRight_->sizeHint();
				scrollRight_->setGeometry(layoutPos, 0, h.width(), h.height());
				layoutPos += h.width() + gap_;
				scrollRight_->setVisible(true);
			}
			{
				QSize h = moreButton_->sizeHint();
				moreButton_->setGeometry(layoutPos, 0, h.width(), h.height());
				layoutPos += h.width() + gap_;
				moreButton_->setVisible(true);
			}
		}
		else {
			scrollRight_->setVisible(false);
			moreButton_->setVisible(false);
		}
		
		{
			QSize h = closeButton_->sizeHint();
			closeButton_->setGeometry(layoutPos, 0, h.width(), h.height());
		}
	}
	
	// carrier makes gaps appear in correct color
	{
		QRect r(0, 0, width(), style_->minHeight());
		if (r != carrier_->geometry())
			carrier_->setGeometry(r);
	}
	
	setUpdatesEnabled(true);
}

int QxTabBar::indexByPos(QPoint pos) const
{
	int index = -1;
	for (int i = 0; i < tabs_.count(); ++i)
		if (tabs_.at(i)->isVisible() && tabs_.at(i)->geometry().contains(pos)) {
			index = i;
			break;
		}
	return index;
}

void QxTabBar::resizeEvent(QResizeEvent* event)
{
	layoutWidgets();
	if (isVisible() && (indexActive_ != -1)) {
		if (((indexAlpha_ <= indexActive_) && (indexActive_ <= indexBeta_)) || (indexBeta_ == 0)) {
			keepInView();
			layoutWidgets();
		}
	}
	update();
	QWidget::resizeEvent(event);
}

void QxTabBar::mousePressEvent(QMouseEvent* event)
{
	dragIndex_ = -1;
	dropIndex_ = -1;
	if (event->buttons() & Qt::LeftButton) {
		int index = indexByPos(event->pos());
		if (index != -1) {
			dragStartPos_ = event->pos();
			if (index != indexActive_)
				activate(index);
			dragIndex_ = index;
		}
	}
}

void QxTabBar::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		dragIndex_ = -1;
		dropMark_->setVisible(false);
	}
}

void QxTabBar::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		if ((dragIndex_ != -1) && (dropIndex_ == -1)) {
			if ((dragStartPos_ - event->pos()).manhattanLength() >= QApplication::startDragDistance())
			{
				QMimeData* md = new QMimeData;
				md->setData(internalMimeFormat(), "");
				QDrag* drag = new QDrag(this);
				drag->setMimeData(md);
				drag->exec(Qt::MoveAction);
				// dragMove(event->pos());
			}
		}
	}
}

void QxTabBar::dragMove(QPoint pos)
{
	QSize h = dropMark_->sizeHint();
	dropIndex_ = -1;
	
	for (int i = 0; i < tabs_.count(); ++i) {
		QxControl* tab = tabs_.at(i);
		if (!tab->isVisible()) continue;
		if ((tab->x() <= pos.x()) && (pos.x() < tab->x() + tab->width())) {
			if (pos.x() < tab->x() + tab->width() / 2) {
				dropMark_->setGeometry(tab->x() - h.width() / 2 - gap_, style_->minHeight() - h.height(), h.width(), h.height());
				dropIndex_ = i;
			}
			else {
				dropMark_->setGeometry(tab->x() + tab->width() - h.width() / 2, style_->minHeight() - h.height(), h.width(), h.height());
				dropIndex_ = i + 1;
			}
			break;
		}
		if (i == 0) {
			if (pos.x() < tab->x()) {
				dropMark_->setGeometry(tab->x() - h.width() / 2 - gap_, style_->minHeight() - h.height(), h.width(), h.height());
				dropIndex_ = 0;
				break;
			}
		}
		if (i == tabs_.count() - 1) {
			if (tab->x() + tab->width() <= pos.x()) {
				dropMark_->setGeometry(tab->x() + tab->width() - h.width() / 2, style_->minHeight() - h.height(), h.width(), h.height());
				dropIndex_ = tabs_.count();
				break;
			}
		}
	}
	if (tabs_.count() == 0) {
		dropMark_->setGeometry(-h.width() / 2, style_->minHeight() - h.height(), h.width(), h.height());
		dropIndex_ = 0;
	}
	if (dropIndex_ != -1) {
		if (!dropMark_->isVisible()) {
			dropMark_->setVisible(true);
			dropMark_->raise();
		}
		if ((dropIndex_ != indexActive_) && (dropIndex_ != indexActive_ + 1))
			dropMark_->move(dropMark_->pos() - QPoint(styleManager()->constant("tabWidgetDropMarkDisplacement"), 0));
	}
	else {
		if (dropMark_->isVisible())
			dropMark_->setVisible(false);
	}
}

void QxTabBar::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(internalMimeFormat())) {
		QxTabBar* source = qobject_cast<QxTabBar*>(event->source());
		bool semanticReject = false;
		if (semanticType() != QString())
			semanticReject = source->semanticType() != semanticType();
		if (!semanticReject) {
			event->acceptProposedAction();
			dropIndex_ = -1;
		}
	}
	else if ((event->mimeData()->hasFormat("text/uri-list")) && acceptUriDrops())  {
		QList<QUrl> urls = event->mimeData()->urls();
		bool reject = false;
		for (int i = 0; i < urls.count(); ++i) {
			if (urls.at(i).isValid()) {
				if (urls.at(i).scheme() != "file") {
					reject = true;
					break;
				}
			}
		}
		if (!reject)
			event->acceptProposedAction();
	}
}

void QxTabBar::leaveEvent(QEvent* event)
{
	dropMark_->setVisible(false);
}

void QxTabBar::dragMoveEvent(QDragMoveEvent* event)
{
	dragMove(event->pos());
}

void QxTabBar::dropEvent(QDropEvent* event)
{
	dropMark_->setVisible(false);
	if (event->mimeData()->hasFormat(internalMimeFormat()))
	{
		QxTabBar* source = qobject_cast<QxTabBar*>(event->source());
		
		if ((!source) || (dropIndex_ == -1))
			return;
		
		if (source == this) {
			moveTab(dragIndex_, dropIndex_);
		}
		else {
			QxControl* tab = source->tabs_.at(source->dragIndex_);
			QWidget* widget = source->widgets_.at(source->dragIndex_);
			
			QString label = tab->visual()->text();
			QPixmap icon = tab->visual()->leadingIcon();
			source->closeTab(source->dragIndex_, false);
			openTab(dropIndex_, label, icon, widget);
		}
		
		dragIndex_ = -1;
		dropIndex_ = -1;
		event->acceptProposedAction();
	}
	else if ((event->mimeData()->hasFormat("text/uri-list")) && acceptUriDrops())
	{
		if (dropIndex_ == -1)
			return;
		QList<QUrl> urls = event->mimeData()->urls();
		for (int i = 0; i < urls.count(); ++i) {
			if (urls.at(i).isValid())
				emit openRequest(dropIndex_, urls.at(i));
		}
		
		event->acceptProposedAction();
	}
}

bool QxTabBar::eventFilter(QObject* proxy, QEvent* event)
{
	QWidget* widget = qobject_cast<QWidget*>(proxy);
	if (widget) {
		if (event->type() == QEvent::DragEnter) {
			QDragEnterEvent* dude = static_cast<QDragEnterEvent*>(event);
			dragEnterEvent(dude);
			return true;
		}
		else if (event->type() == QEvent::DragMove) {
			QDragMoveEvent* dude = static_cast<QDragMoveEvent*>(event);
			QPoint pos = mapFromGlobal(widget->mapToGlobal(dude->pos()));
			dragMove(pos);
			return true;
		}
		else if (event->type() == QEvent::Drop) {
			QDropEvent* dude = static_cast<QDropEvent*>(event);
			dropEvent(dude);
			return true;
		}
	}
	return QObject::eventFilter(proxy, event);
}

} // namespace pacu
