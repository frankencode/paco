// #include <QDebug> // DEBUG
#include <QApplication>
#include <QClipboard>
#include <QLayout>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QTimer>
#include "QxVisual.hpp"
#include "QxStyle.hpp"
#include "QxControl.hpp"

#define VIDE_CONTROL_DEBUG_INPUTMETHOD 0

#if VIDE_IM_WORKAROUND
#warning "Using workaround for broken input method code."
#endif

namespace vide
{

QxControl::QxControl(QWidget* parent, QxVisual* visual)
	: QWidget(parent),
	  parent_(qobject_cast<QxControl*>(parent)),
	  state_(0),
	  inheritState_(false),
	  menu_(0),
	  menuPopupMode_(UpsideMenuPopup|DownsideMenuPopup|PreferDownsideMenuPopup),
	  cursorPos_(0),
	  blink_(false),
	  blinkTimer_(new QTimer(this)),
	  hasSelection_(false),
	  selectionPos0_(0),
	  selectionPos1_(0),
	  preeditCursor_(0),
	  #if VIDE_IM_WORKAROUND
	  ignoreNextKeyPressEvent_(false),
	  #endif
	  focusPredessor_(0)
{
	setMode(DisplayMode);
	
	if (!visual) visual = new QxVisual;
	setVisual(visual);
	
	setAttribute(Qt::WA_InputMethodEnabled, true);
	
	blinkTimer_->setInterval(500);
	connect(blinkTimer_, SIGNAL(timeout()), this, SLOT(updateBlink()));
}

QxControl::~QxControl() // empty constructor HACK, compiler bug workaround
{}

int QxControl::mode() const { return mode_; }
void QxControl::setMode(int mode) {
	mode_ = mode;
	if ((mode_ == LineEditMode) || (mode_ == TextEditMode))
		setFocusPolicy(Qt::StrongFocus);
	else
		setFocusPolicy(Qt::NoFocus);
}

int QxControl::state() const { return inheritState_ ? parent_->state_ : (state_ | (Inactive * (!isActiveWindow()))); }
bool QxControl::state(State flag) const { return (state() & flag) != 0; }
bool QxControl::inheritState() const { return inheritState_; }
void QxControl::setInheritState(bool on) { inheritState_ = on; }

void QxControl::enable(bool on)
{
	if (on)
		state_ = state_ & (~Disabled);
	else
		state_ = state_ | Disabled;
}

QxVisual* QxControl::visual() const { return visual_; }
void QxControl::setVisual(QxVisual* visual) {
	if (visual_ == visual) return;
	visual_ = visual;
	if (visual_) {
		visual_->control_ = this;
		setSizePolicy(visual_->sizePolicy());
		updateGeometry();
	}
	else {
		setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	}
	updateGeometry();
}

QSize QxControl::sizeHint() const { return (layout()) ? QWidget::sizeHint() : visual_->sizeHint(); }
QSize QxControl::minimumSizeHint() const { return (layout()) ? QWidget::minimumSizeHint() : visual_->minimumSizeHint(); }

QMenu* QxControl::menu() const { return menu_; }

void QxControl::setMenu(QMenu* menu)
{
	if (menu_) {
		// disconnect(menu_, SIGNAL(aboutToShow()), this, SLOT(press()));
		disconnect(menu_, SIGNAL(aboutToHide()), this, SLOT(release()));
	}
	Q_ASSERT(menu);
	menu_ = menu;
	mode_ = MenuMode;
	// connect(menu_, SIGNAL(aboutToShow()), this, SLOT(press()));
	connect(menu_, SIGNAL(aboutToHide()), this, SLOT(release()));
}

int QxControl::menuPopupMode() const { return menuPopupMode_; }
void QxControl::setMenuPopupMode(int mode) { menuPopupMode_ = mode; }

bool QxControl::isPressed() const { return (state() & Pressed) != 0; }
bool QxControl::isUnderMouse() const { return (state() & Hover) != 0; }

int QxControl::cursorPos() const { return cursorPos_ + preeditCursor_; }

QString QxControl::text() const {
	return visual_->text();
}
void QxControl::setText(QString text, int cursorPos) {
	visual_->setText(text);
	if (cursorPos > text.length()) cursorPos = text.length();
	else if (cursorPos < 0) cursorPos = 0;
	cursorPos_ = cursorPos;
	if (hasSelection())
		unselect();
}

void QxControl::clearText() {
	if (!visual_->text().isEmpty()) {
		selectAll();
		deleteSelection();
	}
}

bool QxControl::hasSelection() const {
	return hasSelection_;
}

void QxControl::getSelection(int* si0, int* si1) const {
	*si0 = selectionPos0_;
	*si1 = selectionPos1_;
	if (*si0 > *si1) {
		int h = *si1;
		*si1 = *si0;
		*si0 = h;
	}
}

void QxControl::setSelection(int si0, int si1)
{
	selectionPos0_ = si0;
	selectionPos1_ = si1;
	hasSelection_ = true;
	update();
}

void QxControl::deleteSelection()
{
	int i, n;
	if (selectionPos0_ < selectionPos1_) {
		i = selectionPos0_; n = selectionPos1_ - selectionPos0_;
	}
	else {
		i = selectionPos1_; n = selectionPos0_ - selectionPos1_;
	}
	visual_->text_.replace(i, n, "");
	if ((i < cursorPos_) && (cursorPos_ < i + n)) cursorPos_ = i;
	else if (cursorPos_ >= i + n) cursorPos_ -= n;
	hasSelection_ = false;
	update();
}

void QxControl::clearSelection()
{
	hasSelection_ = false;
	update();
}

QString QxControl::copySelection() const
{
	QString value;
	if (hasSelection()) {
		int si0, si1;
		getSelection(&si0, &si1);
		value = visual_->text_.mid(si0, si1 - si0);
	}
	return value;
}

void QxControl::selectAll() {
	hasSelection_ = true;
	selectionPos0_ = 0;
	selectionPos1_ = visual_->text_.length();
	if ((state() & Focus) != 0) {
		blink_ = true;
		blinkTimer_->start();
	}
	update();
}

void QxControl::updateGeometry()
{
	if (visual_)
	{
		QxStyle* style = visual_->style();
		
		if (style)
		{
			setContentsMargins(style->leftMargin(), style->topMargin(), style->rightMargin(), style->bottomMargin());
			
			int minWidth = style->minWidth();
			int minHeight = style->minHeight();
			int maxWidth = style->maxWidth();
			int maxHeight = style->maxHeight();
			if (minWidth != 0) {
				if (minimumWidth() != minWidth)
					setMinimumWidth(minWidth);
			}
			if (minHeight != 0) {
				if (minimumHeight() != minHeight)
					setMinimumHeight(minHeight);
			}
			if (maxWidth != 0) {
				if (maximumWidth() != maxWidth)
					setMaximumWidth(maxWidth);
			}
			if (maxHeight != 0) {
				if (maximumHeight() != maxHeight)
					setMaximumHeight(maxHeight);
			}
		}
	}
	
	if (updatesEnabled())
		QWidget::updateGeometry();
}

bool QxControl::inFocus() const {
	return inFocus(this);
}

bool QxControl::inFocus(const QWidget* widget) {
	QWidget* proxy = widget->focusProxy();
	return (QApplication::focusWidget() == widget) || ((proxy) ? inFocus(proxy) : false);
}

void QxControl::press()
{
	if (parent_)
		parent_->press();
	if (mode_ == TouchMode) {
		toggle(true);
		toggle(false);
	}
	if (mode_ == PushMode) {
		toggle(true);
		grabMouse();
	}
	else if (mode_ == ToggleMode) {
		toggle(((state() & Pressed) == 0));
	}
	else if (mode_ == RadioMode) {
		toggle(true);
	}
	else if (mode_ == MenuMode) {
		showMenu();
		toggle(true);
	}
	else if ((mode_ == LineEditMode) || (mode_ == TextEditMode)) {
		emit pressed();
		QWidget::setFocus(Qt::MouseFocusReason);
	}
}

void QxControl::release()
{
	if (parent_)
		parent_->release();
	if (mode_ == PushMode) {
		releaseMouse();
		toggle(false);
	}
	else if (mode_ == MenuMode) {
		// hideMenu();
		toggle(false);
	}
}

void QxControl::toggle(bool on)
{
	if (on && ((state_ & Pressed) == 0)) {
		state_ |= Pressed;
		emit pressed();
		emit toggled(on);
		update();
	}
	else if ((!on) && ((state_ & Pressed) != 0)) {
		state_ ^= Pressed;
		emit released();
		emit toggled(false);
		update();
	}
}

void QxControl::setFocus() {
	if (!focusPredessor_)
		focusPredessor_ = QApplication::focusWidget();
	#ifdef QT_MAC_USE_COCOA
	if (focusProxy())
		focusProxy()->setFocus();
	#endif
	QWidget::setFocus();
}

void QxControl::clearFocus() {
	if (focusPredessor_) {
		QxControl* control = qobject_cast<QxControl*>(focusPredessor_);
		if (control)
			control->setFocus();
		else
			focusPredessor_->setFocus();
		focusPredessor_ = 0;
	}
	else
		QWidget::clearFocus();
}

void QxControl::select() {
	if ((state_ & Selected) == 0) {
		emit requestSelection();
		state_ |= Selected;
		emit selected();
		update();
	}
}

void QxControl::unselect() {
	if ((state_ & Selected) != 0) {
		state_ ^= Selected;
		emit unselected();
		update();
	}
}

void QxControl::submit() {
	emit textSubmitted(visual_->text_);
}

void QxControl::copy() {
	if (!hasSelection_) return;
	int si0, si1;
	getSelection(&si0, &si1);
	QApplication::clipboard()->setText(visual_->text_.mid(si0, si1 - si0));
	if ((mode_ & Focus) != 0) {
		blink_ = true;
		blinkTimer_->start();
	}
	update();
}

void QxControl::cut() {
	if (!hasSelection_) return;
	int si0, si1;
	getSelection(&si0, &si1);
	QApplication::clipboard()->setText(visual_->text_.mid(si0, si1 - si0));
	visual_->setText(visual_->text().remove(si0, si1 - si0));
	if (si1 <= cursorPos_)
		cursorPos_ -= si1 - si0;
	hasSelection_ = false;
	if ((mode_ & Focus) != 0) {
		blink_ = true;
		blinkTimer_->start();
	}
	emit textEdited(visual_->text_);
	update();
}

void QxControl::paste() {
	if (hasSelection_) {
		int si0, si1;
		getSelection(&si0, &si1);
		visual_->setText(visual_->text().remove(si0, si1 - si0));
		if (si1 <= cursorPos_)
			cursorPos_ -= si1 - si0;
	}
	QString chunk = QApplication::clipboard()->text();
	visual_->setText(visual_->text().insert(cursorPos_, chunk));
	cursorPos_ += chunk.length();
	if ((mode_ & Focus) != 0) {
		blink_ = true;
		blinkTimer_->start();
	}
	emit textEdited(visual_->text_);
	update();
}

void QxControl::updateBlink()
{
	if ((state() & Focus) == 0) {
		blink_ = false;
		blinkTimer_->stop();
	}
	else {
		blink_ = !blink_;
	}
	update();
}

void QxControl::showMenu()
{
	if (!menu_)
		return;
	if (menu_->isVisible())
		return;
	
	// ensure correct sizeHint() is available
	/*#ifdef Q_WS_MAC
	menu_->setUpdatesEnabled(false);
	menu_->show();
	menu_->hide();
	menu_->setUpdatesEnabled(true);
	#endif*/
	
	QSize sz = menu_->sizeHint();
	QRect sg = QApplication::desktop()->screenGeometry(this);
	// QRect wg = window() ? window()->geometry() : sg;
	
	int x0 = mapToGlobal(QPoint(0, 0)).x();
	int x1 = mapToGlobal(QPoint(width(), 0)).x();
	int y0 = mapToGlobal(QPoint(0, 0)).y();
	int y1 = mapToGlobal(QPoint(0, height())).y() - 1;
	
	int wm = sz.width();
	int hm = sz.height();
	
	bool canPopupUpside = (y0 - hm >= 0) && ((menuPopupMode_ & UpsideMenuPopup) != 0);
	bool canPopupDownside = (y1 + hm < sg.bottom()) && ((menuPopupMode_ & DownsideMenuPopup) != 0);
	
	if (canPopupUpside && canPopupDownside) {
		if (menuPopupMode_ & PreferUpsideMenuPopup)
			canPopupDownside = false;
		else if (menuPopupMode_ & PreferDownsideMenuPopup)
			canPopupUpside = false;
	}
	
	//if (menu_->width() < width())
	//	menu_->resize(width(), menu_->height());
	
	int x = x0;
	if (x0 + wm > sg.right())
		x = x1 - wm;
	
	if (canPopupDownside)
		menu_->move(QPoint(x, y1));
	else
		menu_->move(QPoint(x, y0 - hm));
	menu_->show();
}

void QxControl::hideMenu()
{
	menu_->hide();
}

void QxControl::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		bool hasText = !(visual_->text_.isEmpty() && visual_->description_.isEmpty());
		bool leadingOrTrailingPressed = false;
		if (hasText && visual_->leadingRect_.contains(event->pos())) {
			if (hasFocus())
				clearFocus();
			emit leadingPressed();
			leadingOrTrailingPressed = true;
		}
		else if (hasText && visual_->trailingRect_.contains(event->pos())) {
			if (hasFocus())
				clearFocus();
			emit trailingPressed();
			leadingOrTrailingPressed = true;
		}
		if ((!leadingOrTrailingPressed) || ((mode_ != LineEditMode) && (mode_ != TextEditMode))) {
			press();
			if ((state() & Focus) != 0) {
				int ti = -1;
				visual_->draw(0, width(), height(), state(), event->x(), event->y(), &ti);
				if (ti != -1) {
					if ((event->modifiers() & Qt::ShiftModifier) != 0) {
						if (!hasSelection_)
							selectionPos0_ = cursorPos_;
						hasSelection_ = true;
						selectionPos1_ = ti;
					}
					else {
						hasSelection_ = false;
					}
					cursorPos_ = ti;
					
					blink_ = true;
					blinkTimer_->start();
					update();
				}
				else {
					if (hasSelection_) {
						hasSelection_ = false;
						update();
					}
				}
			}
		}
	}
	// event->setAccepted(mode() != DisplayMode);
	QWidget::mousePressEvent(event);
}

void QxControl::mouseReleaseEvent(QMouseEvent* event)
{
	if ((event->button() == Qt::LeftButton) && (mode_ != MenuMode)) release();
	// event->setAccepted(mode() != DisplayMode);
	QWidget::mouseReleaseEvent(event);
}

void QxControl::mouseMoveEvent(QMouseEvent* event)
{
	if (((mode_ == LineEditMode) || (mode_ == TextEditMode)) && (visual_)) {
		if (visual_->textRect_.contains(event->pos()))
			QApplication::changeOverrideCursor(Qt::IBeamCursor);
		else
			QApplication::changeOverrideCursor(cursor());
		if ((event->buttons() & Qt::LeftButton) != 0) {
			int ti = -1;
			visual_->draw(0, width(), height(), state(), event->x(), event->y(), &ti, true);
			if ((ti != -1) && (ti != cursorPos_)) {
				if (!hasSelection_) {
					hasSelection_ = true;
					selectionPos0_ = cursorPos_;
				}
				selectionPos1_ = ti;
				cursorPos_ = ti;
				
				blink_ = true;
				blinkTimer_->start();
				update();
			}
		}
	}
	emit hover();
	event->accept();
	QWidget::mouseMoveEvent(event);
}

void QxControl::mouseDoubleClickEvent(QMouseEvent* event)
{
	QWidget::mouseDoubleClickEvent(event); // generate a mousePressEvent()
	emit doubleClicked();
}

void QxControl::contextMenuEvent(QContextMenuEvent* event)
{
	emit contextMenuRequest(event->pos());
	event->ignore(); // propagate to parent widget
}

void QxControl::wheelEvent(QWheelEvent* event)
{
	if (event->delta() > 0) emit rotate();
	else if (event->delta() < 0) emit backRotate();
	if (!hasFocus()) setFocus();
	QWidget::wheelEvent(event);
}

void QxControl::enterEvent(QEvent* event)
{
	if ((mode_ == LineEditMode) || (mode_ == TextEditMode)) {
		QApplication::setOverrideCursor(cursor());
		setMouseTracking(true);
	}
	if ((state_ & Hover) == 0) {
		state_ |= Hover;
		emit entered();
		update();
	}
}

void QxControl::leaveEvent(QEvent* event)
{
	if ((mode_ == LineEditMode) || (mode_ == TextEditMode)) {
		QApplication::restoreOverrideCursor();
		setMouseTracking(false);
	}
	if ((state_ & Hover) != 0) {
		state_ ^= Hover;
		emit left();
		update();
	}
}

void QxControl::focusInEvent(QFocusEvent* event)
{
	state_ |= Focus;
	blink_ = true;
	blinkTimer_->start();
	emit gotFocus();
	update();
}

void QxControl::focusOutEvent(QFocusEvent* event)
{
	state_ = state_ & ~Focus;
	blink_ = false;
	blinkTimer_->stop();
	emit lostFocus();
	update();
}

void QxControl::keyPressEvent(QKeyEvent* event)
{
	#if VIDE_IM_WORKAROUND
	if (ignoreNextKeyPressEvent_) {
		ignoreNextKeyPressEvent_ = false;
		return;
	}
	#endif
	
	QString text = visual_->text();
	QString textSaved = text;
	int cursorPosSaved = cursorPos_;
	QString chunk = event->text();
	int key = event->key();
	bool ctrl =
		#ifdef Q_WS_MAC
			(event->modifiers() & Qt::MetaModifier) != 0;
		#else
			(event->modifiers() & Qt::ControlModifier) != 0;
		#endif
	bool cmd = (event->modifiers() & Qt::ControlModifier) != 0;
	bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;
	bool alt = (event->modifiers() & Qt::AltModifier) != 0;
	bool plain = !(ctrl || cmd || shift || alt);
	
	// workaround HACK
	// behavior: widget in toolbar loses focus if escape is pressed,
	//           and escape key will be delivered after losing focus!
	if ((!hasFocus()) && (key == Qt::Key_Escape))
		setFocus();
	
	if ( (plain && (key == Qt::Key_Tab)) ||
	     ((mode_ != TextEditMode) && (key == Qt::Key_Up)) ) {
		emit rotate();
		return;
	}
	else if ( (shift && (key == Qt::Key_Backtab)) || // logical HACK, "shift" and "back"?
	          ((mode_ != TextEditMode) && (key == Qt::Key_Down)) ) {
		emit backRotate();
		return;
	}
	else if (key == Qt::Key_Escape) {
		emit dismissed();
		return;
	}
	
	if ((mode_ != LineEditMode) && (mode_ != TextEditMode))
		return;
	
	if ( (key == Qt::Key_Shift) ||
	     (key == Qt::Key_Meta) ||
	     (key == Qt::Key_Control) ||
	     (key == Qt::Key_Alt) ) return;
	
	if (cmd && (key == Qt::Key_C)) {
		copy();
		return;
	}
	if (cmd && (key == Qt::Key_X)) {
		cut();
		return;
	}
	if (cmd && (key == Qt::Key_V)) {
		paste();
		return;
	}
	if ( hasSelection_ && plain &&
	     ( (key == Qt::Key_Backspace) || (key == Qt::Key_Delete) ) ) {
		int si0, si1;
		getSelection(&si0, &si1);
		bool backStep = (cursorPos_ > 0) && (si0 == cursorPos_);
		cursorPos_ -= backStep;
		visual_->setText(visual_->text().remove(si0 - backStep, (si1 - si0) + backStep));
		if (si1 <= cursorPos_)
			cursorPos_ -= si1 - si0;
		hasSelection_ = false;
		emit textEdited(visual_->text_);
		return;
	}
	
	if ((plain || shift) && (key == Qt::Key_Left)) {
		if (cursorPos_ > 0)
			--cursorPos_;
	}
	else if ((plain || shift) && (key == Qt::Key_Right)) {
		if (cursorPos_ < text.length())
			++cursorPos_;
	}
	else if (key == Qt::Key_Up) {
		int ti;
		visual_->draw(0, width(), height(), state(), visual_->cursorPos_.x(), visual_->cursorPos_.y() - visual_->lineHeight_ + 1, &ti, false);
		if (ti != -1)
			cursorPos_ = ti;
	}
	else if (key == Qt::Key_Down) {
		int ti;
		visual_->draw(0, width(), height(), state(), visual_->cursorPos_.x(), visual_->cursorPos_.y() + visual_->lineHeight_ + 1, &ti, false);
		if (ti != -1)
			cursorPos_ = ti;
	}
	else if ( ((plain || shift) && (key == Qt::Key_Home)) ||
	          (ctrl && ((key == Qt::Key_A) || (key == Qt::Key_Left))) )
	{
		while (cursorPos_ > 0) {
			if (text.at(cursorPos_ - 1) == '\n') break;
			--cursorPos_;
		}
	}
	else if ( ((plain || shift) && (key == Qt::Key_End)) ||
	          (ctrl && ((key == Qt::Key_E) || (key == Qt::Key_Right))) )
	{
		while (cursorPos_ < text.length()) {
			if (text.at(cursorPos_) == '\n') break;
			++cursorPos_;
		}
	}
	else if (plain && (key == Qt::Key_Backspace)) {
		if (cursorPos_ > 0) {
			--cursorPos_;
			text.remove(cursorPos_, 1);
		}
	}
	else if (plain && (key == Qt::Key_Delete)) {
		if (cursorPos_ < text.length())
			text.remove(cursorPos_, 1);
	}
	else if (plain && (key == Qt::Key_Return)) {
		if ((mode_ == LineEditMode) || ((mode_ == TextEditMode) && ctrl)) {
			emit textSubmitted(text);
			emit submitted();
		}
		else
			chunk = "\n";
	}
	
	bool printable = (chunk.length() > 0);
	for (int i = 0; (i < chunk.size()) && printable; ++i)
		printable = printable && (chunk.at(i).isPrint() || (chunk.at(i) == '\n'));
	
	if (printable) {
		if (hasSelection_) {
			int si0, si1;
			getSelection(&si0, &si1);
			text.remove(si0, si1 - si0);
			if (si1 <= cursorPos_)
				cursorPos_ -= si1 - si0;
			hasSelection_ = false;
		}
		text.insert(cursorPos_, chunk);
		cursorPos_ += chunk.length();
	}
	else if (/*(cursorPos_ != cursorPosSaved) &&*/ shift) {
		if (!hasSelection_)
			selectionPos0_ = cursorPosSaved;
		selectionPos1_ = cursorPos_;
		hasSelection_ = (selectionPos0_ != selectionPos1_);
	}
	else if (cmd && (key == Qt::Key_A)) {
		hasSelection_ = true;
		selectionPos0_ = 0;
		selectionPos1_ = text.length();
		cursorPos_ = selectionPos1_;
	}
	else {
		hasSelection_ = false;
	}
	
	blink_ = true;
	blinkTimer_->start();
	update();
	
	if (textSaved != text) {
		visual_->setText(text);
		emit textEdited(text);
	}
}

void QxControl::paintEvent(QPaintEvent* event)
{
	if (visual_) {
		QxPainter p(this);
		visual_->draw(&p, width(), height(), state());
	}
}

QVariant QxControl::inputMethodQuery(Qt::InputMethodQuery query) const
{
	#if VIDE_CONTROL_DEBUG_INPUTMETHOD
	QString queryName = "<Unknown query type>";
	#endif
	QVariant reply;
	switch (query) {
		case Qt::ImMicroFocus:
			#if VIDE_CONTROL_DEBUG_INPUTMETHOD
			queryName = "Qt::ImMicroFocus";
			#endif
			reply = QRect(visual_->cursorPos_, visual_->cursorSize_);
			break;
		case Qt::ImFont:
			#if VIDE_CONTROL_DEBUG_INPUTMETHOD
			queryName = "Qt::ImFont";
			#endif
			reply = visual_->style_->font();
			break;
		case Qt::ImCursorPosition:
			#if VIDE_CONTROL_DEBUG_INPUTMETHOD
			queryName = "Qt::ImCursorPosition";
			#endif
			reply = cursorPos_;
			break;
		case Qt::ImSurroundingText:
			#if VIDE_CONTROL_DEBUG_INPUTMETHOD
			queryName = "Qt::ImSurroundingText";
			#endif
			reply = visual_->text_;
			break;
		case Qt::ImCurrentSelection:
			#if VIDE_CONTROL_DEBUG_INPUTMETHOD
			queryName = "Qt::ImCurrentSelection";
			#endif
			reply = copySelection();
			break;
		case Qt::ImMaximumTextLength:
			#if VIDE_CONTROL_DEBUG_INPUTMETHOD
			queryName = "Qt::ImMaximumTextLength";
			#endif
			reply = QVariant(); // no limit
			break;
		case Qt::ImAnchorPosition: {
			#if VIDE_CONTROL_DEBUG_INPUTMETHOD
			queryName = "Qt::ImAnchorPosition";
			#endif
			int si0 = 0, si1 = 0;
			if (hasSelection())
				getSelection(&si0, &si1);
			reply = si0;
			break;
		}
		default:
			reply = QVariant();
	}
	
	#if VIDE_CONTROL_DEBUG_INPUTMETHOD
	qDebug() << "View::inputMethodQuery(): queryName, reply = " << queryName << "," << reply;
	#endif
	
	return reply;
}

void QxControl::inputMethodEvent(QInputMethodEvent* event)
{
	bool beginInput = (event->preeditString() != preeditString_);
	bool finishInput = (!event->commitString().isEmpty()) || (event->replacementLength() > 0);
	bool modifying = (beginInput && hasSelection()) || finishInput;
	
	QString text = visual_->text_;
	QString textSaved = text;
	
	if (!preeditString_.isEmpty())
		text.remove(cursorPos_, preeditString_.length());
	
	if (modifying)
		deleteSelection();
	
	for (int i = 0, n = event->attributes().size(); i < n; ++i)
	{
		const QInputMethodEvent::Attribute& a = event->attributes().at(i);
		if (a.type == QInputMethodEvent::Selection)
			setSelection(a.start, a.start + a.length);
		else if (a.type == QInputMethodEvent::Cursor)
			preeditCursor_ = a.start * (a.length > 0); // quick HACK, unclear semantics
	}
	
	if (finishInput) {
		cursorPos_ += event->replacementStart();
		if (event->replacementLength() > 0)
			text.remove(cursorPos_, event->replacementLength());
		if (!event->commitString().isEmpty()) {
			text.insert(cursorPos_, event->commitString());
			cursorPos_ += event->commitString().length();
		}
		preeditString_ = QString();
		preeditCursor_ = 0;
		#if VIDE_IM_WORKAROUND
		ignoreNextKeyPressEvent_ = true;
		#endif
	}
	else {
		preeditString_ = event->preeditString();
		if (!preeditString_.isEmpty())
			visual_->text_.insert(cursorPos_, preeditString_);
	}
	
	if (textSaved != text)
		visual_->setText(text);
	
	if (finishInput)
		emit textEdited(text);
	
	blink_ = true;
	blinkTimer_->start();
	update();
	
	event->accept();
}

} // namespace vide
