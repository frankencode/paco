#ifndef PTE_DOCUMENT_HPP
#define PTE_DOCUMENT_HPP

#include <QString>
#include <QFileInfo>
#include <QTextStream>
#include "UseFtl.hpp"
#include "Atoms.hpp"
#include "Highlighter.hpp"
// #include "InsightModel.hpp"

namespace pte
{

class MatchingLine: public Instance
{
public:
	MatchingLine()
	{}
	MatchingLine(Document* document, int y, Ref<MatchList, Owner> matches)
		: document_(document),
		  y_(y),
		  matches_(matches)
	{}
	Ref<Document, Owner> document_;
	int y_;
	Ref<MatchList, Owner> matches_;
};

typedef CustomList< Ref<MatchingLine, Owner> > MatchingLines;

class MatchStatus: public Instance, public virtual BackRefList {
public:
	MatchStatus(QRegExp pattern, Ref<MatchingLines, Owner> matchingLines, int yMax, int numFiles, int numMatches)
		: pattern_(pattern),
		  matchingLines_(matchingLines),
		  yMax_(yMax),
		  numFiles_(numFiles),
		  numMatches_(numMatches)
	{}
	QRegExp pattern_;
	Ref<MatchingLines, Owner> matchingLines_;
	int yMax_;
	int numFiles_;
	int numMatches_;
};

class DocumentColorizer;
class InsightModel;

class Document: public QObject, public Instance, public virtual BackRefList
{
	Q_OBJECT
	
public:
	Document(int initialLines = 1);
	~Document();
	
	Ref<Cache> cache() const;
	int numberOfLines() const;
	int lineNumber(int lineIndex) const;
	int lineNumberNumDigits() const;
	
	Ref<Style> defaultStyle() const;
	void setDefaultStyle(Ref<Style> style);
	
	int tabWidth() const;
	void setTabWidth(int value);
	#ifdef QT_MAC_USE_COCOA
	int tabWidth(int x) const;
	#endif
	void updateTabWidths(int y);
	
	bool historyEnabled() const;
	void setHistoryEnabled(bool on);
	
	bool emissionEnabled() const;
	void setEmissionEnabled(bool on);
	
	void begin(QString viewState);
	void end(QString viewState);
	int classifyLastDelta(int* y0, int* y1) const;
	
	void pushLine(int y);
	void popLine(int y);
	void pushSpan(int y, int x0, int* x1, const QString& text, Ref<Style> style = 0, Ref<Token> token = 0);
	void popSpan(int y, int x0, int x1, QString* text = 0);
	
	int lineLength(int y);
	
	enum HistoryDir { Undo = 0, Redo = 1 };
	bool hasPast() const;
	bool hasFuture() const;
	bool hasHistory(int dir = Undo) const;
	QString stepHistory(int dir = Undo, bool forget = false);
	
	void push(int ya, int xa, int* yb, int* xb, const QString& text);
	void pop(int ya, int xa, int yb, int xb, QString* text = 0);
	
	void getCell(int y, int x, QString* ch, Ref<Style>* style = 0, Ref<Token>* token = 0);
	QString getChar(int y, int x);
	QString copySpan(int y, int x0, int x1, char gap = ' ', bool replaceTabBySpace = false);
	QString copy(int ya, int xa, int yb, int xb);
	//QString replaceTabBySpace(QString s, int x0 = 0) const;
	
	void load(QTextStream* source);
	void save(QTextStream* sink);
	bool isModified() const;
	void markDirty();
	
	QString text();
	void setText(const QString& text);
	
	QString title() const;
	void setTitle(QString value);
	
	QString filePath() const;
	void setFilePath(QString value);
	
	QFileInfo fileInfo();
	
	int maxLineLength() const;
	void setMaxLineLength(int value);
	
	bool maxLineLengthExceeded() const;
	
	Ref<Highlighter> highlighter() const;
	void setHighlighter(Ref<Highlighter> highlighter);
	int yaInvalid() const;
	void setYaInvalid(int ya);
	
	// lowlevel exposure (needed for DocumentColorizer)
	int breakChunk(int y, int x);
	void pushChunk(int y, int ci, int x0, int* x1, QString text, Ref<Style> style, Ref<Token> token);
	
	bool mapCursorToContentPos(int cy, int cx, int* k, int* i);
	void stepBackward(int* cy, int* cx);
	void stepForward(int* cy, int* cx);
	void stepNearest(int* cy, int* cx);
	bool beginOfDocument(int cy, int cx) const;
	bool endOfDocument(int cy, int cx) const;
	
	Ref<MatchStatus, Owner> find(QRegExp pattern);
	Ref<MatchStatus, Owner> replace(QRegExp pattern, QString substitute, int* cy = 0, int* cx = 0);
	static Ref<Document, Owner> matchStatusDocument(Ref<MatchStatus> status);
	static void updateMatchStatusDocument(Ref<Document> mdoc, Ref<Delta> delta, Ref<Highlighter> highlighter = 0);
	Ref<MatchStatus> matchStatus() const;
	
	inline void highlighterYield() const { if (highlighter_) highlighter_->yield(); }
	inline void highlighterResume() const { if (highlighter_) highlighter_->resume(); }
	inline void highlighterRestart() const { if (highlighter_) highlighter_->restart(); }
	inline bool highlighterSync(Time timeout = 0) const { return (highlighter_) ? highlighter_->sync(timeout) : true; }
	
	Ref<InsightModel> insightModel() const;
	void setInsightModel(Ref<InsightModel> model);
	
signals:
	void hasFutureChanged(bool);
	void hasPastChanged(bool);
	void isModifiedChanged(bool value);
	void changed(Ref<Delta, Owner> delta);
	void highlighterReady();
	
private:
	Delta* newBegin(QString viewState);
	Delta* newEnd(QString viewState);
	Delta* newDelta(int type, int y, int x0 = -1, int x1 = -1, QString text = QString());
	
	void setHasPast(bool value);
	void setHasFuture(bool value);
	
	Ref<Line, Owner> cloneLine(int y);
	Ref<MatchList, Owner> matchLine(int y, QRegExp pattern);
	Ref<MatchStatus, Owner> findAndReplace(QRegExp pattern, bool replace, QString substitute = QString(), int* cy = 0, int* cx = 0);

	static void matchStatusYield(Ref<MatchStatus> status, bool sync = false);
	static void matchStatusResume(Ref<MatchStatus> status);
	
	Ref<Cache, Owner> cache_;
	Ref<Style, Owner> defaultStyle_;
	
	Ref<Log, Owner> past_, future_;
	bool hasPast_, hasFuture_;
	int timeSaved_;
	
	mutable Ref<Highlighter, Owner> highlighter_;
	int yaInvalid_; // first line of which validity of highlighting state is unsure
	
	Ref<MatchStatus, SetNull> matchStatus_;
	
	bool historyEnabled_;
	bool emissionEnabled_;
	int tabWidth_;
	
	QString title_;
	QString filePath_;
	QFileInfo fileInfo_;
	
	int maxLineLength_;
	bool maxLineLengthExceeded_;
	
	Ref<InsightModel, Owner> insightModel_;
};

} // namespace pte

#endif // PTE_DOCUMENT_HPP
