#include <QDebug> // DEBUG
#include <QApplication>
#include <QTextLayout>
#include <QPainter>
#include "SearchPatternHighlighter.hpp"

namespace pte
{

SearchPatternHighlighter::SearchPatternHighlighter(QObject* parent)
	: QStyledItemDelegate(parent)
{}

void SearchPatternHighlighter::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItemV4 optionV4Copy = option;
	QStyleOptionViewItemV4* optionV4 = &optionV4Copy;
	initStyleOption(optionV4, index);
	
	const QWidget* widget = optionV4->widget;
	QStyle* style = widget ? widget->style() : QApplication::style();
	QString text = optionV4->text;
	QRect rect = style->subElementRect(QStyle::SE_ItemViewItemText, optionV4, widget);
	const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;
	rect = rect.adjusted(textMargin, 0, -textMargin, 0); // style HACK, see QCommonStyle
	optionV4->text = "";
	
	style->drawControl(QStyle::CE_ItemViewItem, optionV4, painter, widget);
	
	painter->save();
	painter->setPen(
		option.palette.color(
			(option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text
		)
	);
	{
		QTextLayout layout(text, optionV4->font);
		layout.beginLayout();
		QTextLine line = layout.createLine();
		line.setLineWidth(rect.width());
		line.setPosition(QPointF(0, 0));
		qreal textHeight = line.height();
		layout.endLayout();
		if (pattern_ != "") {
			int i = text.toLower().indexOf(pattern_);
			if (i != -1) {
				QTextCharFormat format;
				format.setFontWeight(QFont::Bold);
				QTextLayout::FormatRange range;
				range.start = i;
				range.length = pattern_.length();
				range.format = format;
				layout.setAdditionalFormats(QList<QTextLayout::FormatRange>() << range);
			}
		}
		layout.draw(
			painter,
			QPointF(rect.x(), rect.y() + (rect.height() - textHeight) / 2.)
		);
	}
	painter->restore();
	
}

void SearchPatternHighlighter::setPattern(const QString& pattern)
{
	pattern_ = pattern.toLower();
}

} // namespace pte
