#ifndef PACU_QXWRAPPERSTYLE_HPP
#define PACU_QXWRAPPERSTYLE_HPP

#include <QStyle>
#include <QPointer>

namespace pacu
{

class QxWrapperStyle: public QStyle
{
	Q_OBJECT
	
public:
	QxWrapperStyle(QStyle* original);
	
	virtual void polish(QWidget *);
	virtual void unpolish(QWidget *);
	
	virtual void polish(QApplication *);
	virtual void unpolish(QApplication *);
	
	virtual void polish(QPalette &);
	
	virtual QRect itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled, const QString &text) const;
	virtual QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const;
	
	virtual void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const;
	
	virtual void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const;
	
	virtual QPalette standardPalette() const;
	
	virtual void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const;
	virtual void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const;
	
	virtual QRect subElementRect(SubElement subElement, const QStyleOption *option, const QWidget *widget = 0) const;
	
	
	virtual void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *widget = 0) const;
	virtual SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pt, const QWidget *widget = 0) const;
	virtual QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget = 0) const;
	
	virtual int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
	
	virtual QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *w = 0) const;
	
	virtual int styleHint(StyleHint stylehint, const QStyleOption *opt = 0, const QWidget *widget = 0, QStyleHintReturn* returnData = 0) const;
	
	virtual QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt = 0, const QWidget *widget = 0) const;
	
	QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = 0, const QWidget *widget = 0) const;
	
	virtual QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const;
	
	int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option = 0, const QWidget *widget = 0) const;
	int combinedLayoutSpacing(QSizePolicy::ControlTypes controls1, QSizePolicy::ControlTypes controls2, Qt::Orientation orientation, QStyleOption *option = 0, QWidget *widget = 0) const;
	
	QStyle* original() const;
	
protected Q_SLOTS:
	QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0, const QWidget *widget = 0) const;
	int layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option = 0, const QWidget *widget = 0) const;
private:
	QPointer<QStyle> original_;
};

} // namespace pacu

#endif // PACU_QXWRAPPERSTYLE_HPP
