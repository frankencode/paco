#include "TextElider.hpp"

namespace pte
{

TextElider::TextElider(QWidget* widget, Qt::TextElideMode mode)
	: QObject(widget),
	  widget_(widget),
	  mode_(mode)
{}

void TextElider::setText(const QString& s)
{
	if (widget_) {
		int width = 0;
		if (widget_->isVisible()) {
			width = widget_->contentsRect().width();
		}
		else {
			int l, t, r, b;
			widget_->getContentsMargins(&l, &t, &r, &b);
			width = widget_->sizeHint().width() - l - r;
		}
		emit textChanged(
			widget_->fontMetrics().elidedText(s, mode_, width)
		);
	}
}

} // namespace pte
