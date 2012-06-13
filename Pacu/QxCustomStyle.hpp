#ifndef PACU_QXCUSTOMSTYLE_HPP
#define PACU_QXCUSTOMSTYLE_HPP

#include <QObject>
#include <QProxyStyle>

namespace pacu
{

class QxCustomStyle: public QProxyStyle
{
	Q_OBJECT
	
public:
	QxCustomStyle();
	
#ifdef Q_WS_X11
	virtual int pixelMetric(PixelMetric metric, const QStyleOption* option = 0, const QWidget* widget = 0) const;
#endif
	virtual void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = 0) const;
	
protected slots:
	QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption* option = 0, const QWidget* widget = 0) const;

private:
#ifdef Q_WS_MAC
	QIcon critical_;
	QIcon information_;
	QIcon question_;
#endif
	QColor borderColor_;
};

} // namespace pacu

#endif // PACU_QXCUSTOMSTYLE_HPP
