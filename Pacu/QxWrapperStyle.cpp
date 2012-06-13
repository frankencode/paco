#include "QxWrapperStyle.hpp"

namespace pacu
{

QxWrapperStyle::QxWrapperStyle(QStyle* original)
	: original_(original)
{
	original_->setParent(this);
}

void QxWrapperStyle::polish(QWidget *w) { original_->polish(w); }
void QxWrapperStyle::unpolish(QWidget *w) { original_->unpolish(w); }

void QxWrapperStyle::polish(QApplication *a) { original_->polish(a); }
void QxWrapperStyle::unpolish(QApplication *a) { original_->unpolish(a); }

void QxWrapperStyle::polish(QPalette &p) { original_->polish(p); }

QRect QxWrapperStyle::itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled, const QString &text) const {
	return original_->itemTextRect(fm, r, flags, enabled, text);
}
QRect QxWrapperStyle::itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const {
	return original_->itemPixmapRect(r, flags, pixmap);
}

void QxWrapperStyle::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const {
	original_->drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}

void QxWrapperStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const {
	original_->drawItemPixmap(painter, rect, alignment, pixmap);
}

QPalette QxWrapperStyle::standardPalette() const {
	return original_->standardPalette();
}

void QxWrapperStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const {
	original_->drawPrimitive(pe, opt, p, w);
}
void QxWrapperStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w) const {
	original_->drawControl(element, opt, p, w);
}

QRect QxWrapperStyle::subElementRect(SubElement subElement, const QStyleOption *option, const QWidget *widget) const {
	return original_->subElementRect(subElement, option, widget);
}

void QxWrapperStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *widget) const {
	original_->drawComplexControl(cc, opt, p, widget);
}

QStyle::SubControl QxWrapperStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pt, const QWidget *widget) const {
	return original_->hitTestComplexControl(cc, opt, pt, widget);
}
QRect QxWrapperStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget) const {
	return original_->subControlRect(cc, opt, sc, widget);
}

int QxWrapperStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const {
	return original_->pixelMetric(metric, option, widget);
}

QSize QxWrapperStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *w) const {
	return original_->sizeFromContents(ct, opt, contentsSize, w);
}

int QxWrapperStyle::styleHint(StyleHint stylehint, const QStyleOption *opt, const QWidget *widget, QStyleHintReturn* returnData) const {
	return original_->styleHint(stylehint, opt, widget, returnData);
}

QPixmap QxWrapperStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget) const {
	return original_->standardPixmap(standardPixmap, opt, widget);
}

QIcon QxWrapperStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const {
	return original_->standardIcon(standardIcon, option, widget);
}

QPixmap QxWrapperStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const {
	return original_->generatedIconPixmap(iconMode, pixmap, opt);
}

int QxWrapperStyle::layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const {
	return original_->layoutSpacing(control1, control2, orientation, option, widget);
}
int QxWrapperStyle::combinedLayoutSpacing(QSizePolicy::ControlTypes controls1, QSizePolicy::ControlTypes controls2, Qt::Orientation orientation, QStyleOption *option, QWidget *widget) const {
	return original_->combinedLayoutSpacing(controls1, controls2, orientation, option, widget);
}

QStyle* QxWrapperStyle::original() const { return original_; }

QIcon QxWrapperStyle::standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt, const QWidget *widget) const {
	return original_->standardIcon(standardIcon, opt, widget);
}

int QxWrapperStyle::layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const {
	return original_->layoutSpacing(control1, control2, orientation, option, widget);
}

} // namespace pacu
