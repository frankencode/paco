#ifndef VIDE_QXCONTROL_HPP
#define VIDE_QXCONTROL_HPP

#include <QWidget>
#include <QMenu>
#include <QPointer>
#include "UseFtl.hpp"

#ifdef Q_WS_MAC
#ifndef QT_MAC_USE_COCOA
#if (0x040500 <= QT_VERSION) && (QT_VERSION < 0x040700)
#define VIDE_IM_WORKAROUND 1
#endif
#endif
#endif
#ifndef VIDE_IM_WORKAROUND
#define VIDE_IM_WORKAROUND 0
#endif

namespace vide
{

class QxVisual;

class QxControl: public QWidget, public virtual Instance, public virtual BackRefList
{
	Q_OBJECT
	
public:
	QxControl(QWidget* parent = 0, QxVisual* visual = 0);
	~QxControl();
	
	enum Mode {
		DisplayMode = 0,
		TouchMode = 1,
		PushMode = 2,
		ToggleMode = 3,
		RadioMode = 4,
		MenuMode = 5,
		LineEditMode = 6,
		TextEditMode = 7
	};
	
	enum State {
		Released = 0,
		Pressed = 1,
		Hover = 2,
		Focus = 4,
		Selected = 8,
		Inactive = 16,
		Disabled = 32,
		NumState = 64
	};
	
	enum MenuPopup {
		UpsideMenuPopup = 1,
		DownsideMenuPopup = 2,
		PreferUpsideMenuPopup = 4,
		PreferDownsideMenuPopup = 8
	};
	
	int mode() const;
	void setMode(int mode);
	
	int state() const;
	bool state(State flag) const;
	bool inheritState() const;
	void setInheritState(bool on);
	
	void enable(bool on);
	
	QxVisual* visual() const;
	void setVisual(QxVisual* visual);
	
	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;
	
	QMenu* menu() const;
	void setMenu(QMenu* menu);
	
	int menuPopupMode() const;
	void setMenuPopupMode(int mode);
	
	bool isPressed() const;
	bool isUnderMouse() const;
	
	int cursorPos() const;
	QString text() const;
	bool hasSelection() const;
	void getSelection(int* si0, int* si1) const;
	void setSelection(int si0, int si1);
	
	bool inFocus() const;
	
signals:
	void pressed();
	void released();
	void toggled(bool on);
	void entered();
	void left();
	void hover();
	void gotFocus();
	void lostFocus();
	void selected();
	void unselected();
	void requestSelection();
	
	void leadingPressed();
	void trailingPressed();
	
	void rotate();
	void backRotate();
	void submitted();
	void dismissed();
	
	void textEdited(QString text);
	void textSubmitted(QString text);
	
	void contextMenuRequest(QPoint pos);
	void doubleClicked();
	
public slots:
	void press();
	void release();
	void toggle(bool on);
	void setFocus();
	void clearFocus();
	void select();
	void unselect();
	
	void submit();
	void copy();
	void cut();
	void paste();
	
	void setText(QString text, int cursorPos = intMax);
	void clearText();
	void selectAll();
	void deleteSelection();
	void clearSelection();
	QString copySelection() const;
	
	void updateGeometry();
	
private slots:
	void updateBlink();
	
protected:
	void showMenu();
	void hideMenu();
	
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);
	virtual void contextMenuEvent(QContextMenuEvent* event);
	virtual void wheelEvent(QWheelEvent* event);
	virtual void enterEvent(QEvent* event);
	virtual void leaveEvent(QEvent* event);
	virtual void focusInEvent(QFocusEvent* event);
	virtual void focusOutEvent(QFocusEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void paintEvent(QPaintEvent* event);
	virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
	virtual void inputMethodEvent(QInputMethodEvent* event);
	
private:
	static bool inFocus(const QWidget* widget);
	void establishSizeContraints();
	
	friend class QxVisual;
	
	QxControl* parent_;
	int mode_;
	int state_, stateSaved_;
	bool inheritState_;
	Ref<QxVisual, Owner> visual_;
	QPointer<QMenu> menu_;
	int menuPopupMode_;
	
	int cursorPos_;
	bool blink_;
	QTimer* blinkTimer_;
	
	bool hasSelection_;
	int selectionPos0_, selectionPos1_;
	
	QString preeditString_;
	int preeditCursor_;
	#if VIDE_IM_WORKAROUND
	bool ignoreNextKeyPressEvent_;
	#endif
	
	QWidget* focusPredessor_;
};

} // namespace vide

#endif // VIDE_QXCONTROL_HPP
