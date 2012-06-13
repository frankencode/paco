#ifndef PTE_SEARCHPATTERNHIGHLIGHTER_HPP
#define PTE_SEARCHPATTERNHIGHLIGHTER_HPP

#include <QStyledItemDelegate>

namespace pte
{

class SearchPatternHighlighter: public QStyledItemDelegate
{
	Q_OBJECT
	
public:
	SearchPatternHighlighter(QObject* parent = 0);
	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

public slots:
	void setPattern(const QString& pattern);
	
private:
	QString pattern_;
};

} // namespace pte

#endif // PTE_SEARCHPATTERNHIGHLIGHTER_HPP
