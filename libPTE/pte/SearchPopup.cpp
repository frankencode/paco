// #include <QDebug>
#include <QCoreApplication>
#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QKeyEvent>
#include <QInputMethodEvent>
#include <QTimer>
#include <QLayout>
#include <QListView>
#include <QAbstractItemModel>
#include "SearchPatternHighlighter.hpp"
#include "SearchPopup.hpp"

namespace pte
{

SearchPopup::SearchPopup(QWidget* widget, QPoint pos, QAbstractItemModel* model, QWidget* quickInfo, QString pattern)
	: QFrame(0, Qt::Popup),
	  widget_(widget),
	  window_(widget->window()),
	  listView_(new QListView(this)),
	  quickInfo_(quickInfo),
	  origMousePos_(QCursor::pos()),
	  preeditString_(pattern),
	  preeditCursor_(pattern.length()),
	  commited_(false)
{
	setAttribute(Qt::WA_DeleteOnClose);
	
	widget_->installEventFilter(this);
	window_->installEventFilter(this);
	
	listView_->viewport()->installEventFilter(this);
	listView_->viewport()->setMouseTracking(true);
	listView_->setCursor(Qt::PointingHandCursor);
	listView_->setTextElideMode(Qt::ElideMiddle);
	listView_->setModel(model);
	
	SearchPatternHighlighter* patternHighlighter = new SearchPatternHighlighter(listView_);
	listView_->setItemDelegate(patternHighlighter);
	connect(this, SIGNAL(searchPatternChanged(const QString&)), patternHighlighter, SLOT(setPattern(const QString&)));
	
	if (!model->parent())
		model->setParent(listView_);
	listView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	listView_->setSelectionMode(QAbstractItemView::SingleSelection);
	connect(listView_, SIGNAL(clicked(const QModelIndex&)), this, SLOT(commit(const QModelIndex&)));
	connect(
		listView_->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
		this, SLOT(currentChanged(const QModelIndex&, const QModelIndex&))
	);
	
	#ifdef QT_MAC_USE_COCOA
	setFrameStyle(QFrame::NoFrame);
	#else
	setFrameStyle(QFrame::Plain|QFrame::StyledPanel);
	#endif
	listView_->setFrameStyle(QFrame::NoFrame);
	
	QVBoxLayout* col = new QVBoxLayout;
	col->setSpacing(0);
	col->setMargin(0);
	col->addWidget(listView_);
	if (quickInfo_) {
		QFrame* hl = new QFrame;
		hl->setFrameStyle(QFrame::Raised|QFrame::HLine);
		col->addWidget(hl);
		col->addWidget(quickInfo_);
		quickInfo_->setMaximumWidth(listView_->sizeHint().width()); // quick HACK
	}
	setLayout(col);
	
	listView_->setFocusPolicy(Qt::NoFocus);
		// ensure list view is not grabbing keyboard input and is not showing focus rectangle
	
	connect(
		this, SIGNAL(searchPatternChanged(const QString&)),
		this, SLOT(selectItemByPattern(const QString&))
	);
	
	QTimer::singleShot(0, this, SLOT(selectItem()));
	// QTimer::singleShot(0, this, SLOT(updateInputContext()));
	move(widget->mapToGlobal(pos));
	show();
}

void SearchPopup::selectItem(int index)
{
	if (index == -1) {
		selectItem(0);
		selectItemByPattern(preeditString_);
		updateInputContext();
		// preeditString_ = "";
	}
	else {
		QModelIndex modelIndex = listView_->model()->index(index, 0);
		if (modelIndex.isValid()) {
			listView_->selectionModel()->select(modelIndex, QItemSelectionModel::Current|QItemSelectionModel::Select);
			listView_->setCurrentIndex(modelIndex);
		}
	}
}

void SearchPopup::selectItemByPattern(const QString& pattern)
{
	QAbstractItemModel* model = listView_->model();
	QModelIndex i = model->index(0, 0);
	while (i.isValid()) {
		QString s = model->data(i, Qt::EditRole).toString();
		if (s.left(pattern.length()).compare(pattern, Qt::CaseInsensitive) == 0) break;
		i = model->index(i.row() + 1, 0);
	}
	if (!i.isValid()) {
		i = model->index(0, 0);
		QString patternToLower = pattern.toLower();
		while (i.isValid()) {
			QString s = model->data(i, Qt::EditRole).toString();
			if (s.toLower().contains(patternToLower)) break;
			i = model->index(i.row() + 1, 0);
		}
	}
	if (i.isValid()) selectItem(i.row());
}

void SearchPopup::commit(const QModelIndex& index)
{
	preeditString_ = listView_->model()->data(index, Qt::EditRole).toString();
	preeditCursor_ = preeditString_.length();
	updateInputContext(true);
	commited_ = true;
	QTimer::singleShot(0, this, SLOT(close()));
}

void SearchPopup::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
	emit currentChanged(current);
}

void SearchPopup::updateInputContext(bool commit)
{
	QInputMethodEvent inputEvent(
		preeditString_,
		QList<QInputMethodEvent::Attribute>()
			<< QInputMethodEvent::Attribute(
				QInputMethodEvent::Cursor,
				preeditCursor_, 1,
				preeditCursor_
			)
	);
	if (commit) {
		if (preeditString_.length() > 0) {
			inputEvent.setCommitString(preeditString_);
			preeditString_ = "";
			preeditCursor_ = 0;
			updateInputContext();
		}
	}
	emit searchPatternChanged(preeditString_);
	listView_->viewport()->update();
	if (widget_)
		QCoreApplication::sendEvent(widget_, &inputEvent);
}

int SearchPopup::selectedItem() const
{
	QItemSelection selection = listView_->selectionModel()->selection();
	return (selection.length() == 0) ? -1 : selection.at(0).top();
}

bool SearchPopup::eventFilter(QObject* obj, QEvent* event)
{
	bool filter = false;
	if (obj == widget_) {
		if (event->type() == QEvent::FocusOut) {
			filter = true; // keep focus displayed on underlying widget
		}
		else if ( (event->type() == QEvent::MouseButtonPress) ||
		          (event->type() == QEvent::Wheel) ||
		          ((event->type() == QEvent::InputMethod) && (event->spontaneous())) ) {
			close();
		}
	}
	else if (obj == window_) {
		if ( (event->type() == QEvent::ApplicationActivate) ||
		     (event->type() == QEvent::ApplicationDeactivate) ||
		     (event->type() == QEvent::MouseButtonPress) ||
		     (event->type() == QEvent::Move) ||
		     (event->type() == QEvent::Resize) ||
		     (event->type() == QEvent::Shortcut) /*||
		     (event->type() == QEvent::WindowActivate) ||
		     (event->type() == QEvent::WindowDeactivate)*/ ) {
			close();
		}
	}
	else if (obj == listView_->viewport()) {
		if (event->type() == QEvent::MouseMove) {
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if ((origMousePos_ - mouseEvent->globalPos()).manhattanLength() >= QApplication::startDragDistance()) {
				QMouseEvent changed(
					QEvent::MouseMove,
					mouseEvent->pos(),
					(mouseEvent->button() != Qt::NoButton) ? mouseEvent->button() : Qt::LeftButton,
					mouseEvent->buttons() | Qt::LeftButton, mouseEvent->modifiers()
				);
				*mouseEvent = changed;
			}
		}
	}
	return filter;
}

void SearchPopup::keyPressEvent(QKeyEvent* event)
{
	int key = event->key();
	if ((key == Qt::Key_Return) || (key == Qt::Key_Tab)) {
		commit(listView_->currentIndex());
	}
	else if (key == Qt::Key_Escape) {
		close();
	}
	else if ( (key == Qt::Key_Up) ||
	          (key == Qt::Key_Down) ||
	          (key == Qt::Key_PageUp) ||
	          (key == Qt::Key_PageDown) ) {
		if (event->spontaneous())
			QCoreApplication::sendEvent(listView_, event);
	}
	else if (key == Qt::Key_Backspace) {
		if (preeditCursor_ > 0) {
			preeditString_ =
				preeditString_.mid(0, preeditCursor_-1) +
				preeditString_.mid(preeditCursor_, preeditString_.length() - preeditCursor_);
			--preeditCursor_;
			updateInputContext();
		}
	}
	else if (key == Qt::Key_Delete) {
		if (preeditCursor_ < preeditString_.length()) {
			preeditString_ =
				preeditString_.mid(0, preeditCursor_) +
				preeditString_.mid(preeditCursor_ + 1, preeditString_.length() - preeditCursor_ - 1);
			updateInputContext();
		}
	}
	else if ((key == Qt::Key_Left) || (key == Qt::Key_Right)) {
		int delta =
			- (key == Qt::Key_Left) * (preeditCursor_ > 0)
			+ (key == Qt::Key_Right) * (preeditCursor_ < preeditString_.length());
		if (delta != 0) {
			preeditCursor_ += delta;
			updateInputContext();
		}
	}
	else {
		QString s = event->text();
		if (s.length() > 0) {
			bool printable = true;
			for (int i = 0; (i < s.length()) && printable; ++i)
				printable = s.at(i).isPrint();
			if (printable) {
				bool identifier = true;
				for (int i = 0; (i < s.length()) && identifier; ++i) {
					QChar ch = s.at(i);
					identifier =
						(('a' <= ch) && (ch <= 'z')) ||
						(('A' <= ch) && (ch <= 'Z')) ||
						(('0' <= ch) && (ch <= '9')) ||
						(ch == '_') || (ch == '!');
				}
				if (identifier) {
					preeditString_ =
						preeditString_.mid(0, preeditCursor_) +
						s +
						preeditString_.mid(preeditCursor_, preeditString_.length() - preeditCursor_);
					preeditCursor_ += s.length();
					updateInputContext();
				}
				else {
					commit(listView_->currentIndex());
					QApplication::sendEvent(widget_, event);
				}
			}
		}
	}
}

void SearchPopup::closeEvent(QCloseEvent* event)
{
	if ((preeditString_ != "") || (preeditCursor_ != 0)) {
		preeditString_ = "";
		preeditCursor_ = 0;
		updateInputContext();
	}
	if (commited_) emit accepted();
	else emit rejected();
}


} // namespace pte
