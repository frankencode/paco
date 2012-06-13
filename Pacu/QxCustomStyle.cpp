#include <QApplication>
#include <QStyleOption>
#include <QDebug>
#include "UseVide.hpp"
#include "QxCustomStyle.hpp"

namespace pacu
{

#define SKY_ADD_PIXMAP(name) \
	{ \
		QPixmap pm; \
		pm.loadFromData(name, name ## Size); \
		name ## _.addPixmap(pm); \
	}

QxCustomStyle::QxCustomStyle()
	: QProxyStyle(qApp->style())
{
#ifdef Q_WS_MAC
	{
		#include "sky/32x32/critical.c"
		#include "sky/32x32/information.c"
		#include "sky/32x32/question.c"
		SKY_ADD_PIXMAP(critical);
		SKY_ADD_PIXMAP(information);
		SKY_ADD_PIXMAP(question);
	}
	{
		#include "sky/48x48/critical.c"
		#include "sky/48x48/information.c"
		#include "sky/48x48/question.c"
		SKY_ADD_PIXMAP(critical);
		SKY_ADD_PIXMAP(information);
		SKY_ADD_PIXMAP(question);
	}
#endif
	borderColor_ = styleManager()->color("border");
}

#ifdef Q_WS_X11
int QxCustomStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
	if (metric == PM_DockWidgetSeparatorExtent) return 1;
	// if (metric == QStyle::PM_DockWidgetHandleExtent) return 1;
	/*else if (metric == QStyle::PM_LayoutHorizontalSpacing) return 7;
	else if (metric == QStyle::PM_LayoutVerticalSpacing) return 7;*/
	return QProxyStyle::pixelMetric(metric, option, widget);
}
#endif

void QxCustomStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
	if (element == PE_IndicatorDockWidgetResizeHandle)
		painter->fillRect(option->rect, borderColor_);
	else
		QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QIcon QxCustomStyle::standardIconImplementation(StandardPixmap standardIcon, const QStyleOption* option, const QWidget* widget) const
{
	QIcon icon;
#ifdef Q_WS_MAC
	/*if (standardIcon == QStyle::SP_MessageBoxCritical)
		icon = critical_;
	else*/ if (standardIcon == QStyle::SP_MessageBoxInformation)
		icon = information_;
	else if (standardIcon == QStyle::SP_MessageBoxQuestion)
		icon = question_;
	else
#endif
		icon = QProxyStyle::standardIconImplementation(standardIcon, option, widget);
	return icon;
}

} // namespace pacu
