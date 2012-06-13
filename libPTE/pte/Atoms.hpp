#ifndef PTE_ATOMS_HPP
#define PTE_ATOMS_HPP

#include <QString>
#include <QColor>
#include <QTextLayout>
#include <QPixmap>
#include <ftl/Token.hpp>
#include "UseFtl.hpp"
#include "CustomList.hpp"

namespace pte
{

class TextLayout: public QTextLayout, public Instance, public virtual BackRefList
{
public:
	TextLayout(const QString& text, const QFont& font, QPaintDevice* device)
		: QTextLayout(text, font, device)
	{}
};

class Pixmap: public QPixmap, public Instance, public virtual BackRefList
{
public:
	Pixmap(int width, int height)
		: QPixmap(width, height)
	{}
};

class Style: public Instance
{
public:
	Style()
		: fgColor_(Qt::black),
		  bgColor_(Qt::white),
		  bold_(false),
		  underline_(false)
	{}
	
	Style(QColor fgColor, QColor bgColor, bool bold = false, bool underline = false)
		: fgColor_(fgColor),
		  bgColor_(bgColor),
		  bold_(bold),
		  underline_(underline)
	{}
	
	QColor fgColor_;
	QColor bgColor_;
	bool bold_;
	bool underline_;
};

class Chunk: public Instance
{
public:
	Chunk(const QString& text, int x0, int x1, Ref<Style> style, Ref<Token> token)
		: text_(text),
		  x0_(x0),
		  x1_(x1),
		  style_(style),
		  token_(token)
	{}
	QString text_;
	int x0_, x1_;
	Ref<Style, Owner> style_;
	Ref<Token, Owner> token_;
};

typedef CustomList< Ref<Chunk, Owner> > ChunkList;

class Link: public Instance, public virtual BackRefList
{
public:
	enum Type { FilePos, Uri };
	Link(int type, int x0, int x1, QString targetPath, int targetRow = -1, int targetColumn = -1)
		: type_(type),
		  x0_(x0),
		  x1_(x1),
		  targetPath_(targetPath),
		  targetRow_(targetRow),
		  targetColumn_(targetColumn)
	{}
	int type_;
	int x0_;
	int x1_;
	QString targetPath_;
	int targetRow_;
	int targetColumn_;
};

typedef CustomList< Ref<Link, Owner> > LinkList;

class Match
{
public:
	Match() {}
	Match(int x0, int x1): x0_(x0), x1_(x1) {}
	int x0_, x1_;
};

typedef CustomList<Match> MatchList;

class Line: public Instance
{
public:
	Line(int tabWidth)
		: chunkList_(new ChunkList),
		  dirty_(false),
		  tabWidth_(tabWidth),
		  numFragments_(-1),
		  wx_(-1),
		  yr_(0),
		  i0Hint_(-1)
	{
		chunkList_->enableIndexCaching();
	}
	
	inline int width() const {
		return (chunkList_->length() > 0) ? chunkList_->get(-1)->x1_ : 0;
	}
	
	inline int tabWidth(int x) const {
		return tabWidth_ - (x % tabWidth_);
	}
	
	QString replaceTabBySpace(QString s, int x0) const
	{
		int i = 0, n = s.size(), x = x0;
		QStringList l;
		while (i < n) {
			int k = i;
			while (k < n) {
				if (s.at(k) == '\t') break;
				++k;
			}
			if (i < k) {
				QString h = s.mid(i, k-i);
				l.append(h);
				x += h.size();
			}
			if (k != n) {
				QString h(tabWidth(x), ' ');
				l.append(h);
				x += h.size();
			}
			i = k + 1;
		}
		return l.join(QString());
	}
	
	Ref<ChunkList, Owner> chunkList_;
	Ref<LinkList, Owner> linkList_;
	Ref<Pixmap, SetNull> pixmap_;
	Ref<TextLayout, SetNull> layout_;
	Ref<MatchList, SetNull> matches_;
	bool dirty_;
	int tabWidth_;
	int numFragments_;
	int wx_;
	int yr_;
	int i0Hint_;
};

typedef CustomList< Ref<Line, Owner> > Cache;

class Document;

class Delta: public Instance
{
public:
	enum Type { Begin = 0, End = 1, PushLine = 2, PopLine = 4, PushSpan = 8, PopSpan = 16 };
	Ref<Document, SetNull> document_;
	int type_;
	int y_, x0_, x1_, tw_;
	QString text_;
	QString viewState_;
};

typedef CustomList< Ref<Delta, Owner> > Log;

} // namespace pte

#endif // PTE_ATOMS_HPP
