#ifndef PTE_CONTEXT_HPP
#define PTE_CONTEXT_HPP

#include <QObject>
#include <QPointer>
#include <codetips/codetips.hpp>
#include "UseFtl.hpp"
#include "Edit.hpp" // HACK, to please moc

namespace pte
{

class Context: public QObject, public codetips::Context
{
	Q_OBJECT
	
public:
	Context(Edit* edit);
	~Context();
	
	String path() const;
	String language() const;
	
	String text() const;
	String copyLine(int line) const;
	int numberOfLines() const;
	String indent() const;
	String indentOf(int line) const;
	
	int bytePos() const;
	int pos() const;
	int line() const;
	int linePos() const;
	
	void insert(String text);
	void move(int delta);
	void select(int delta);
	String copy(int delta) const;
	
private slots:
	void changed(Ref<Delta, Owner> delta);
	
private:
	static String string(const QString& s);
	void cursorByteAndCharOffset(off_t* byteOffset = 0, off_t* charOffset = 0) const;
	
	QPointer<Edit> edit_;
	mutable String text_;
	mutable String cachedLineText_;
	mutable int cachedLine_;
};

} // namespace pte

#endif // PTE_EDITCONTEXT_HPP
