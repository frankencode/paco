#ifndef PTE_EDIT_HPP
#define PTE_EDIT_HPP

#include "View.hpp"

#ifdef Q_WS_MAC
#ifndef QT_MAC_USE_COCOA
#if (0x040500 <= QT_VERSION) && (QT_VERSION < 0x040700)
#define PTE_IM_WORKAROUND 1
#endif
#endif
#endif
#ifndef PTE_IM_WORKAROUND
#define PTE_IM_WORKAROUND 0
#endif

class QMenu;

namespace pte
{

class Context;
class SearchPopup;

class Edit: public View
{
	Q_OBJECT
	
public:
	Edit(QWidget* parent = 0);
	~Edit();
	
	static Ref<ViewMetrics, Owner> defaultMetrics();
	
	bool autoIndent() const;
	QString indent() const;
	
	QSize sizeHint() const;
	
	virtual int cursorLine() const;
	virtual int cursorColumn() const;
	virtual bool moveCursor(int cy, int cx);
	
	void insert(const QString& text);
	void move(int deltaChars);
	void select(int deltaChars);
	QString copy(int deltaChars);
	
	static QMenu* setupActions(QWidget* parent);
	
	Ref<Context> context();
	
signals:
	void cursorMoved(int cy, int cx);
	
public slots:
	void setIndent(QString value);
	void setAutoIndent(bool on);
	
	void undo();
	void redo();
	void copy();
	void cut();
	void paste();
	void paste(QString text, bool moveCursor = true);
	void selectWord();
	void selectLine();
	void selectAll();
	void duplicateLine();
	
private slots:
	void undoForget();
	
private:
	friend class Context;
	
	void stepCursor(int* cy, int* cx, int deltaChars);
	void undoRedo(int dir, bool forget = false);
	void copyToClipboard();
	void deleteSelection();
	void cutSelection();
	void pasteAtCursor();
	void pasteAtCursor(QString text, bool moveCursor = true);
	
	virtual bool hasBracketMatch() const;
	virtual void getBracketMatch(int* bmy0, int* bmx0, int* bmy1, int* bmx1);
	void matchBrackets();
	void matchBrackets(int y, int x);
	
	void beginEdit();
	void endEdit();
	void beginModify();
	void endModify();
	void assistantBeginEdit();
	void assistantBeginModify();
	void assistantEnd();
	
	virtual bool isWord(QChar ch) const;
	void stepPreviousWord(int* cy, int* cx);
	void stepNextWord(int* cy, int* cx);
	
	bool getCodetips(QKeyEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void inputMethodEvent(QInputMethodEvent* event);
	
	int cy_, cx_; // cursor position
	int cy2_, cx2_; // saved cursor position
	int cxSaved_; // saved column for stepping up/down
	int needFullUpdate_;
	
	int vya2_, vxa2_, vyb2_, vxb2_; // saved view range
	bool tinyDoc2_;
	
	bool hasBracketMatch_;
	int bmy0_, bmx0_; // opening bracket pos
	int bmy1_, bmx1_; // closing bracket pos
	
	bool autoIndent_;
	QString indent_;
	
	Ref<Style, Owner> preeditStyle_;
	QString preeditString_;
	int preeditCursor_;
	#if PTE_IM_WORKAROUND
	bool ignoreNextKeyPressEvent_;
	#endif
	
	Ref<Context, Owner> context_;
	QPointer<SearchPopup> searchPopup_;
	
	bool assistantEdit_;
	bool assistantModify_;
	
	bool eatenTabBefore_;
	bool searchPopupIsModifying_;
};

} // namespace pte

#endif // PTE_EDIT_HPP
