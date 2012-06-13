#include <QStyle>
#if QT_VERSION >= 0x040500
#include <QStyleOptionTabV3>
#include <QStyleOptionTabBarBaseV2>
#endif
#include <QStyleOptionHeader>
#include <QApplication>
#include "UseVide.hpp"
#include "QxPacuTheme.hpp"

namespace pacu
{

static QColor gray(qreal value, qreal alpha = 1.0) { return QColor::fromRgbF(value, value, value, alpha); }

static void core()
{
	{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("panel", style);
		#ifdef Q_WS_MAC
		if (QSysInfo::MacintoshVersion == QSysInfo::MV_TIGER)
		#endif
			style->setBackground(new QxTexture(QColor::fromRgbF(0.9, 0.9, 0.9)));
	}
	{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("popup", style);
		style->setBackground(new QxTexture(QColor::fromRgbF(0.9, 0.9, 0.9)));
	}
	{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("finish", style);

		style->setFontSizePx(13);
		{
			QFontMetrics fm(style->font());
			int h = fm.ascent() + fm.descent();
			h += 7;
			style->setMinHeight(h);
		}
		#ifdef Q_WS_X11
		style->setTextDisplacement(1);
		#endif
		//style->setMinHeight(19);
		{
			QLinearGradient gradient(0, 0, 0, 1);
			gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
			const double a = 0.99, b = 0.95, c = 0.9;
			gradient.setColorAt(0, QColor::fromRgbF(a, a, a));
			gradient.setColorAt(0.449, QColor::fromRgbF(b, b, b));
			gradient.setColorAt(0.45, QColor::fromRgbF(c, c, c));
			gradient.setColorAt(1, QColor::fromRgbF(c, c, c));
			style->setBackground(new QxTexture(gradient));
		}
		style->setEngraving(true);
		style->setEngravingColor(QColor::fromRgbF(1., 1., 1.));
	}

	styleManager()->setColor("border", gray(0.67));

	{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("border", style);
		style->setBackground(new QxTexture(styleManager()->color("border")));
	}

	{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("workspaceSplitter", style);
		styleManager()->setConstant("workspaceSplitterWidth", 9);

		class Bg: public QxTexture {
		public:
			virtual void draw(QxPainter* p, int x, int y, int w, int h) {
				{
					QLinearGradient gradient(0, 0, 0, 1);
					gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
					gradient.setColorAt(0, QColor::fromRgbF(1., 1., 1.));
					gradient.setColorAt(1, QColor::fromRgbF(0.85, 0.85, 0.85));
					p->fillRect(0, 0, w, h, gradient);
				}
				{
					QStyleOption so;
					so.initFrom(p->widget());
					#ifdef Q_WS_X11
					so.state = so.state & (~QStyle::State_MouseOver);
					#endif
					so.rect = QRect(x, y, w, h);
					p->style()->drawControl(QStyle::CE_Splitter, &so, p, p->widget());
				}
			}
		};
		style->setBackground(new Bg);
	}

	{
		QxStyle* style = new QxStyle(styleManager()->style("finish"));
		styleManager()->setStyle("workspaceSwitch", style);

		style->setLeftMargin(10);
		style->setRightMargin(style->leftMargin());
		style->setLeadingSpace(7);

		{
			#include "m0n0/toolOpen.c"
			QPixmap black; black.loadFromData(toolOpen, toolOpenSize);
			#include "m0n0/toolOpenNegativ.c"
			QPixmap white; white.loadFromData(toolOpenNegativ, toolOpenNegativSize);

			QPixmap open = QPixmap(black.width(), black.height() + 1);
			open.fill(Qt::transparent);
			QPainter p(&open);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.75);
			p.drawPixmap(0, 0, black);
			style->setLeadingPatch(open, QxControl::Pressed);
		}
		{
			#include "m0n0/toolClosed.c"
			QPixmap black; black.loadFromData(toolClosed, toolClosedSize);
			#include "m0n0/toolClosedNegativ.c"
			QPixmap white; white.loadFromData(toolClosedNegativ, toolClosedNegativSize);

			QPixmap closed = QPixmap(black.size());
			closed.fill(Qt::transparent);
			QPainter p(&closed);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.75);
			p.drawPixmap(0, 0, black);
			style->setLeadingPatch(closed);
		}
	}
	{
		QxStyle* style = new QxStyle(styleManager()->style("finish"));
		styleManager()->setStyle("languageButton", style);
		style->setTrailingSpace(5);
		style->setLeftMargin(10);
		style->setRightMargin(10);

		#include "m0n0/popup.c"
		QPixmap black;
		black.loadFromData(popup, popupSize);

		QPixmap pressed(black.width(), black.height());
		{
			pressed.fill(Qt::transparent);
			QPainter p(&pressed);
			p.setOpacity(1.);
			p.drawPixmap(0, 0, black);
		}

		QPixmap released(black.width(), black.height());
		{
			released.fill(Qt::transparent);
			QPainter p(&released);
			p.setOpacity(0.8);
			p.drawPixmap(0, 0, black);
		}

		style->setTrailingPatch(released);
	}
	{
		QxStyle* style = new QxStyle(styleManager()->style("finish"));
		styleManager()->setStyle("gotoLineButton", style);
		#include "m0n0/gotoLine.c"
		QPixmap black; black.loadFromData(gotoLine, gotoLineSize);
		#include "m0n0/gotoLineNegativ.c"
		QPixmap white; white.loadFromData(gotoLineNegativ, gotoLineNegativSize);
		{
			QPixmap released(black.width(), black.height() + 1);
			released.fill(Qt::transparent);
			QPainter p(&released);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.8);
			p.drawPixmap(0, 0, black);
			style->setLeadingPatch(released);
		}
		{
			QPixmap pressed(black.width(), black.height() + 1);
			pressed.fill(Qt::transparent);
			QPainter p(&pressed);
			p.drawPixmap(0, 1, white);
			// p.setOpacity(0.8);
			p.drawPixmap(0, 0, black);
			style->setLeadingPatch(pressed, QxControl::Hover);
		}
		style->setLeadingSpace(7);
		style->setLeftMargin(10);
		style->setRightMargin(0);
	}
	{
		QxStyle* style = new QxStyle(styleManager()->style("finish"));
		styleManager()->setStyle("resizeHandleGap", style);
		style->setMinWidth(14);
	}
}

static bool useGradient() {
	bool on = true;
	#ifdef Q_WS_MAC
	// on = (QSysInfo::MacintoshVersion != QSysInfo::MV_LEOPARD);
	#endif
	return on;
}

static void tabWidget()
{
	styleManager()->setConstant("tabWidgetGap", useGradient());
	#ifdef Q_WS_MAC
	styleManager()->setConstant("tabWidgetDropMarkDisplacement", useGradient() ? 0 : 2);
	#else
	styleManager()->setConstant("tabWidgetDropMarkDisplacement", useGradient() ? 0 : 1);
	#endif

	{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("tabWidgetBar", style);

		{
			QFont font = style->font();
			#ifdef Q_WS_MAC
			if (!useGradient()) font.setBold(true);
			#endif
			font.setPixelSize(13);
			style->setFont(font);
		}
		if (useGradient()) {
			// style->setTextDisplacement(1);
			style->setEngraving(true);
			style->setEngravingColor(QColor::fromRgbF(1., 1., 1., 0.80));
		}
		else {
			#ifdef Q_WS_MAC
			style->setTextColor(QColor::fromRgbF(0., 0., 0., 0.87));
			style->setTextColor(QColor::fromRgbF(0., 0., 0., 0.83), QxControl::Pressed);
			style->setTextColor(QColor::fromRgbF(0., 0., 0., 0.82), QxControl::Inactive);
			style->setTextColor(QColor::fromRgbF(0., 0., 0., 0.77), QxControl::Inactive|QxControl::Pressed);
			style->setEngraving(true);
			style->setEngravingColor(QColor::fromRgbF(1., 1., 1., 0.35));
			style->setTextDisplacement(1);
			/*style->setTopMargin(1);
			style->setBottomMargin(1);*/
			#endif
			#ifdef Q_WS_X11
			style->setTopMargin(4); // quick HACK for QGtkStyle
			// style->setTextDisplacement(1);
			#endif
		}

		#ifdef Q_WS_MAC
		if (useGradient())
			style->setMinHeight(22);
		else
			style->setMinHeight(23);
		#endif
		#ifdef Q_WS_X11
		if (useGradient())
			style->setMinHeight(22);
		else
			style->setMinHeight(25);
		#endif

		style->setMaxHeight(style->minHeight());
	}

	styleManager()->setStyle("tabWidgetVoid", styleManager()->style("panel"));

	if (useGradient()) {
		styleManager()->setStyle("tabWidgetCarrier", styleManager()->style("border"));
	}
	else {
		#if QT_VERSION >= 0x040500
		QxStyle* style = new QxStyle(styleManager()->style("tabWidgetBar"));
		styleManager()->setStyle("tabWidgetCarrier", style);

		{
			class Bg: public QxTexture {
			public:
				virtual void draw(QxPainter* p, int x, int y, int w, int h) {
					QStyleOptionTabBarBaseV2 so;
					so.initFrom(p->widget());
					so.rect = QRect(x, y, w, h);
					so.documentMode = true;
					p->style()->drawPrimitive(QStyle::PE_FrameTabBarBase, &so, p, p->widget());
					#ifdef Q_WS_MAC
					if (p->widget()->property("tabWidgetCarrierLeadingHighlight").toBool())
						p->fillRect(0, 2, 1, h - 4, QColor::fromRgbF(1., 1., 1., 0.2));
					else
						p->fillRect(w - 1, 2, 1, h - 4, QColor::fromRgbF(1., 1., 1., 0.2));
					#endif
				}
			};
			style->setBackground(new Bg);
		}
		#endif
	}

	{
		QxStyle* style = new QxStyle(styleManager()->style("tabWidgetBar"));
		styleManager()->setStyle("tabWidgetTab", style);

		style->setMinWidth(100); // usability HACK

		if (useGradient())
		{
			{
				class Bg: public QxTexture {
				public:
					virtual void draw(QxPainter* p, int x, int y, int w, int h) {
						{
							QLinearGradient gradient(0, 0, 0, 1);
							gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
							#ifdef Q_WS_MAC
							gradient.setColorAt(0, QColor::fromRgbF(0.77, 0.77, 0.77));
							gradient.setColorAt(1, QColor::fromRgbF(0.92, 0.92, 0.92));
							#endif
							#ifdef Q_WS_X11
							gradient.setColorAt(0, QColor::fromRgbF(0.86, 0.86, 0.86));
							gradient.setColorAt(1, QColor::fromRgbF(0.93, 0.93, 0.93));
							#endif
							p->fillRect(x, y, w, h - 1, gradient);
						}
						if (p->widget()->isActiveWindow()) {
							p->fillRect(x, y, w, 1, QColor::fromRgbF(1., 1., 1., 0.2));
							p->fillRect(x, y + h - 2, w, 1, QColor::fromRgbF(1., 1., 1., 0.2));
						}
						const int sw = 5, sh = 4; // shadow width, height
						#ifdef Q_WS_X11
						const double si = 0.05; // shadow intensity
						#else
						const double si = 0.07; // shadow intensity
						#endif
						{	// top shadow
							QLinearGradient gradient(0, 0, 0, 1);
							gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
							gradient.setColorAt(0, QColor::fromRgbF(0., 0., 0., si));
							gradient.setColorAt(1, QColor::fromRgbF(0., 0., 0., 0.));
							p->fillRect(x + sw, y, w - 2 * sw, sh, gradient);
						}
						{	// top left corner shadow
							QRadialGradient gradient(1, 1, 1);
							gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
							gradient.setColorAt(0, QColor::fromRgbF(0., 0., 0., 0.));
							gradient.setColorAt(1, QColor::fromRgbF(0., 0., 0., si));
							p->fillRect(x, y, sw, sh, gradient);
						}
						{	// top right corner shadow
							QRadialGradient gradient(0, 1, 1);
							gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
							gradient.setColorAt(0, QColor::fromRgbF(0., 0., 0., 0.));
							gradient.setColorAt(1, QColor::fromRgbF(0., 0., 0., si));
							p->fillRect(x + w - sw, y, sw, sh, gradient);
						}
						{	// left shadow
							QLinearGradient gradient(0, 0, 1, 0);
							gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
							gradient.setColorAt(0, QColor::fromRgbF(0., 0., 0., si));
							gradient.setColorAt(1, QColor::fromRgbF(0., 0., 0., 0.));
							p->fillRect(x, y + sh, sw, h - 2 * sh - 1, gradient);
						}
						{	// right shadow
							QLinearGradient gradient(1, 0, 0, 0);
							gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
							gradient.setColorAt(0, QColor::fromRgbF(0., 0., 0., si));
							gradient.setColorAt(1, QColor::fromRgbF(0., 0., 0., 0.));
							p->fillRect(x + w - sw, y + sh, sw, h - 2 * sh - 1, gradient);
						}
						{	// bottom shadow
							QLinearGradient gradient(0, 1, 0, 0);
							gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
							gradient.setColorAt(0, QColor::fromRgbF(0., 0., 0., si));
							gradient.setColorAt(1, QColor::fromRgbF(0., 0., 0., 0.));
							p->fillRect(x + sw, y + h - sh - 1, w - 2 * sw, sh, gradient);
						}
						{	// bottom left corner shadow
							QRadialGradient gradient(1, 0, 1);
							gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
							gradient.setColorAt(0, QColor::fromRgbF(0., 0., 0., 0.));
							gradient.setColorAt(1, QColor::fromRgbF(0., 0., 0., si));
							p->fillRect(x, y + h - sh - 1, sw, sh, gradient);
						}
						{	// bottom right corner shadow
							QRadialGradient gradient(0, 0, 1);
							gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
							gradient.setColorAt(0, QColor::fromRgbF(0., 0., 0., 0.));
							gradient.setColorAt(1, QColor::fromRgbF(0., 0., 0., si));
							p->fillRect(x + w - sw, y + h - sh - 1, sw, sh, gradient);
						}
						p->fillRect(x, y + h - 1, w, 1, QColor::fromRgbF(0.67, 0.67, 0.67));
					}
				};
				style->setBackground(new Bg, QxControl::Pressed);
			}
			{
				class Bg: public QxTexture {
				public:
					virtual void draw(QxPainter* p, int x, int y, int w, int h) {
						QLinearGradient gradient(0, 0, 0, 1);
						gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
						#ifdef Q_WS_MAC
						gradient.setColorAt(0, QColor::fromRgbF(0.92, 0.92, 0.92));
						gradient.setColorAt(1, QColor::fromRgbF(0.77, 0.77, 0.77));
						#endif
						#ifdef Q_WS_X11
						gradient.setColorAt(0, QColor::fromRgbF(0.95, 0.95, 0.95));
						gradient.setColorAt(1, QColor::fromRgbF(0.88, 0.88, 0.88));
						#endif
						p->fillRect(x, y, w, h - 1, gradient);
						if (p->widget()->isActiveWindow())
						#ifdef Q_WS_MAC
							p->fillRect(x, y, w, 1, QColor::fromRgbF(1., 1., 1., 0.3));
						#else
							p->fillRect(x, y, w, 1, QColor::fromRgbF(1., 1., 1., 0.6));
						#endif
						p->fillRect(x, y + h - 1, w, 1, QColor::fromRgbF(0.67, 0.67, 0.67));
					};
				};
				style->setBackground(new Bg);
			}
		}
		else
		{
			#if QT_VERSION >= 0x040500
			{
				class Bg: public QxTexture {
				public:
					virtual void draw(QxPainter* p, int x, int y, int w, int h) {
						QStyleOptionTabV3 so;
						so.initFrom(p->widget());
						#ifdef Q_WS_MAC
						w -= 2; // because QMacStyle seems to overdraw, i.e. workaround HACK of workaround HACK
						#endif
						#ifdef Q_WS_X11
						w -= 1;
						#endif
						so.rect = QRect(x, y, w, h);
						so.documentMode = true;
						so.selectedPosition = QStyleOptionTab::SelectedPosition(p->widget()->property("selectedPosition").toInt());
						so.position = QStyleOptionTab::TabPosition(p->widget()->property("position").toInt());
						p->style()->drawControl(QStyle::CE_TabBarTabShape, &so, p, p->widget());
					}
				};
				style->setBackground(new Bg);
			}
			{
				class Bg: public QxTexture {
				public:
					virtual void draw(QxPainter* p, int x, int y, int w, int h) {
						QStyleOptionTabV3 so;
						so.initFrom(p->widget());
						so.rect = QRect(x, y, w, h);
						so.state |= QStyle::State_Selected;
						so.documentMode = true;
						so.selectedPosition = QStyleOptionTab::SelectedPosition(p->widget()->property("selectedPosition").toInt());
						so.position = QStyleOptionTab::TabPosition(p->widget()->property("position").toInt());
						p->style()->drawControl(QStyle::CE_TabBarTabShape, &so, p, p->widget());
					}
				};
				style->setBackground(new Bg, QxControl::Pressed);
			}
			#endif
		}

		style->setLeftMargin(11);
		style->setRightMargin(11);
		style->setVirtualText(" *");
	}

	{
		QxStyle* style = new QxStyle(styleManager()->style("tabWidgetBar"));
		styleManager()->setStyle("tabWidgetButton", style);
		style->setBackground(styleManager()->style("tabWidgetTab")->background());
		style->setBackground(styleManager()->style("tabWidgetTab")->background(QxControl::Pressed), QxControl::Pressed);
	}

	if (useGradient()) {
		styleManager()->setStyle("tabWidgetFiller", styleManager()->style("tabWidgetButton"));
	}
	else {
		#if QT_VERSION >= 0x040500
		QxStyle* style = new QxStyle(styleManager()->style("tabWidgetBar"));
		styleManager()->setStyle("tabWidgetFiller", style);

		{
			class Bg: public QxTexture {
			public:
				virtual void draw(QxPainter* p, int x, int y, int w, int h) {
					QStyleOptionTabV3 so;
					so.initFrom(p->widget());
					#ifdef Q_WS_MAC
					w -= 2; // because QMacStyle seems to overdraw, i.e. workaround HACK of workaround HACK
					#endif
					#ifdef Q_WS_X11
					w -= 1;
					#endif
					so.state = so.state & (~QStyle::State_MouseOver);
					so.rect = QRect(x, y, w, h);
					so.documentMode = true;
					/*so.selectedPosition = QStyleOptionTab::SelectedPosition(p->widget()->property("selectedPosition").toInt());
					so.position = QStyleOptionTab::TabPosition(p->widget()->property("position").toInt());*/
					p->style()->drawControl(QStyle::CE_TabBarTabShape, &so, p, p->widget());
				}
			};
			style->setBackground(new Bg);
		}
		#endif
	}

	{
		QxStyle* style = new QxStyle(styleManager()->style("tabWidgetButton"));
		styleManager()->setStyle("tabWidgetMoreButton", style);

		#include "m0n0/more.c"
		#include "m0n0/moreNegativ.c"
		QPixmap black; black.loadFromData(more, moreSize);
		QPixmap white; white.loadFromData(moreNegativ, moreNegativSize);

		QPixmap released(black.width(), black.height() + 1);
		released.fill(Qt::transparent);
		{
			QPainter p(&released);
			p.setOpacity(useGradient() ? 0.75 : 0.4);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.7);
			p.drawPixmap(0, 0, black);
		}

		QPixmap pressed(black.width(), black.height() + 1);
		pressed.fill(Qt::transparent);
		{
			QPainter p(&pressed);
			p.drawPixmap(0, 1, white);
			p.drawPixmap(0, 0, black);
		}

		style->setLeadingPatch(released);
		if (useGradient()) {
			style->setLeadingPatch(pressed, QxControl::Pressed);
			style->setLeadingPatch(pressed, QxControl::Hover);
			style->setLeftMargin(0);
			style->setRightMargin(0);
		}
		else {
			style->setMargins(1, 0, 1, 1);
		}
		style->setMinWidth(style->minHeight());
	}

	{
		QxStyle* style = new QxStyle(styleManager()->style("tabWidgetButton"));
		styleManager()->setStyle("tabWidgetScrollLeftButton", style);

		#include "m0n0/scrollLeft.c"
		#include "m0n0/scrollLeftNegativ.c"
		QPixmap black; black.loadFromData(scrollLeft, scrollLeftSize);
		QPixmap white; white.loadFromData(scrollLeftNegativ, scrollLeftNegativSize);

		QPixmap released(black.width(), black.height() + 1);
		released.fill(Qt::transparent);
		{
			QPainter p(&released);
			p.setOpacity(useGradient() ? 1. : 0.4);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.7);
			p.drawPixmap(0, 0, black);
		}

		QPixmap pressed(black.width(), black.height() + 1);
		pressed.fill(Qt::transparent);
		{
			QPainter p(&pressed);
			p.drawPixmap(0, 1, white);
			p.drawPixmap(0, 0, black);
		}

		style->setLeadingPatch(released);
		if (useGradient()) {
			style->setLeadingPatch(pressed, QxControl::Pressed);
			style->setLeadingPatch(pressed, QxControl::Hover);
			style->setLeftMargin(3);
			style->setRightMargin(4);
		}
		else {
			style->setMargins(1, 3, 0, 6);
		}
	}

	{
		QxStyle* style = new QxStyle(styleManager()->style("tabWidgetButton"));
		styleManager()->setStyle("tabWidgetScrollRightButton", style);

		#include "m0n0/scrollRight.c"
		#include "m0n0/scrollRightNegativ.c"
		QPixmap black; black.loadFromData(scrollRight, scrollRightSize);
		QPixmap white; white.loadFromData(scrollRightNegativ, scrollRightNegativSize);

		QPixmap released(black.width(), black.height() + 1);
		released.fill(Qt::transparent);
		{
			QPainter p(&released);
			p.setOpacity(useGradient() ? 1. : 0.4);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.7);
			p.drawPixmap(0, 0, black);
		}

		QPixmap pressed(black.width(), black.height() + 1);
		pressed.fill(Qt::transparent);
		{
			QPainter p(&pressed);
			p.drawPixmap(0, 1, white);
			p.drawPixmap(0, 0, black);
		}

		style->setLeadingPatch(released);
		if (useGradient()) {
			style->setLeadingPatch(pressed, QxControl::Pressed);
			style->setLeadingPatch(pressed, QxControl::Hover);
			style->setLeftMargin(4);
			style->setRightMargin(3);
		}
		else {
			style->setMargins(1, 3, 0, 5);
		}
	}

	{
		QxStyle* style = new QxStyle(styleManager()->style("tabWidgetButton"));
		styleManager()->setStyle("tabWidgetCloseButton", style);

		#include "m0n0/close.c"
		#include "m0n0/closeNegativ.c"
		QPixmap black; black.loadFromData(close, closeSize);
		QPixmap white; white.loadFromData(closeNegativ, closeNegativSize);

		QPixmap released(black.width(), black.height() + 1);
		released.fill(Qt::transparent);
		{
			QPainter p(&released);
			p.setOpacity(useGradient() ? 0.75 : 0.4);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.7);
			p.drawPixmap(0, 0, black);
		}

		QPixmap pressed(black.width(), black.height() + 1);
		pressed.fill(Qt::transparent);
		{
			QPainter p(&pressed);
			p.drawPixmap(0, 1, white);
			p.drawPixmap(0, 0, black);
		}

		style->setLeadingPatch(released);
		if (useGradient()) {
			style->setLeadingPatch(pressed, QxControl::Pressed);
			style->setLeadingPatch(pressed, QxControl::Hover);
		}
		style->setLeftMargin(0);
		style->setRightMargin(0);
		if (!useGradient())
			style->setTopMargin(2);
		style->setMinWidth(style->minHeight());
	}

	{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("tabWidgetDropMark", style);

		static const char* const xpm[] = {
		    "9 5 2 1",
		    "x c #000000",
		    ". c None",
		    "....x....",
			"...xxx...",
			"..xxxxx..",
			".xxxxxxx.",
			"xxxxxxxxx"
		};
		style->setLeadingPatch(QPixmap(xpm));
	}

	if (useGradient()) {
		QxStyle* style = new QxStyle(styleManager()->style("tabWidgetTab"));
		style->setLeftMargin(0);
		style->setRightMargin(0);
		styleManager()->setStyle("tabWidgetToolHeader", style);
	}
	else {
		styleManager()->setStyle("tabWidgetToolHeader", styleManager()->style("tabWidgetCarrier"));
	}

	if (useGradient()) {
		styleManager()->setStyle("tabWidgetToolSplitter", styleManager()->style("border"));
	}
	else {
		#if QT_VERSION >= 0x040500
		QxStyle* style = new QxStyle(styleManager()->style("tabWidgetBar"));
		styleManager()->setStyle("tabWidgetToolSplitter", style);
		{
			class Bg: public QxTexture {
			public:
				virtual void draw(QxPainter* p, int x, int y, int w, int h) {
					{
						QStyleOptionTabBarBaseV2 so;
						so.initFrom(p->widget());
						so.rect = QRect(x, y, w, h);
						so.documentMode = true;
						p->style()->drawPrimitive(QStyle::PE_FrameTabBarBase, &so, p, p->widget());
					}
					{
						QStyleOptionTabV3 so;
						so.initFrom(p->widget());
						so.rect = QRect(x - 3, y, 3, h);
						so.documentMode = true;
						p->style()->drawControl(QStyle::CE_TabBarTabShape, &so, p, p->widget());
					}
				}
			};
			style->setBackground(new Bg);
		}
		#endif
	}
}

static void fileBrowser()
{
	styleManager()->setConstant("dirOpenMenuFontSizePx", 13);
	styleManager()->setConstant("fileBrowserFontSizePx", 12);

	styleManager()->setStyle("fileBrowserNavBar", styleManager()->style("tabWidgetBar"));
	styleManager()->setStyle("fileBrowserNavCarrier", styleManager()->style("tabWidgetToolHeader"));

	{
		QxStyle* style = new QxStyle(styleManager()->style("fileBrowserNavBar"));
		styleManager()->setStyle("fileBrowserGotoButton", style);

		style->setElideMode(Qt::ElideRight);
		style->setSpacings(5, 5);
		style->setLeftMargin(10);
		style->setRightMargin(10);

		#include "m0n0/popup.c"
		QPixmap black;
		black.loadFromData(popup, popupSize);

		QPixmap pressed(black.width(), black.height());
		{
			pressed.fill(Qt::transparent);
			QPainter p(&pressed);
			p.setOpacity(1.);
			p.drawPixmap(0, 0, black);
		}

		QPixmap released(black.width(), black.height());
		{
			released.fill(Qt::transparent);
			QPainter p(&released);
			p.setOpacity(0.7);
			p.drawPixmap(0, 0, black);
		}

		style->setTrailingPatch(released);
		if (useGradient()) {
			style->setTrailingPatch(pressed, QxControl::Hover);
			style->setTrailingPatch(pressed, QxControl::Pressed);
		}
		else {
			class Bg: public QxTexture {
			public:
				virtual void draw(QxPainter* p, int x, int y, int w, int h) {
					p->fillRect(x, y + 2, w, h - 4, QColor::fromRgbF(0., 0., 0., 0.1));
				}
			};
			Bg* bg = new Bg;
			style->setBackground(bg, QxControl::Hover);
			style->setBackground(bg, QxControl::Pressed);
		}
	}
	{
		QxStyle* style = new QxStyle(styleManager()->style("fileBrowserNavBar"));
		styleManager()->setStyle("fileBrowserCdUpButton", style);

		style->setRightMargin(5);
		style->setLeftMargin(5);
		style->setBottomMargin(1);

		#include "m0n0/cdup.c"
		QPixmap pressed;
		pressed.loadFromData(cdup, cdupSize);
		QPixmap released(pressed.width(), pressed.height());
		{
			released.fill(Qt::transparent);
			QPainter p(&released);
			p.setOpacity(0.65);
			p.drawPixmap(0, 0, pressed);
		}

		style->setLeadingPatch(released);
		if (useGradient()) {
			style->setLeadingPatch(pressed, QxControl::Hover);
			style->setLeadingPatch(pressed, QxControl::Pressed);
		}
		else {
			class Bg: public QxTexture {
			public:
				virtual void draw(QxPainter* p, int x, int y, int w, int h) {
					p->fillRect(x, y + 2, w, h - 4, QColor::fromRgbF(0., 0., 0., 0.1));
				}
			};
			Bg* bg = new Bg;
			style->setBackground(bg, QxControl::Hover);
			style->setBackground(bg, QxControl::Pressed);
		}
	}
	/*{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("fileBrowserDirView", style);
		style->setBackground(new QxTexture(QColor("#E6EDF7")));
	}*/
	styleManager()->setColor("fileBrowserDirViewBgColor", QColor("#E6EDF7"));

	{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("fileBrowserSplitter", style);

		style->setFontSizePx(12);
		style->setElideMode(Qt::ElideRight);
		style->setEngraving(true);
		style->setEngravingColor(QColor::fromRgbF(1., 1., 1.));
		#ifdef Q_WS_X11
		style->setTextDisplacement(1);
		#endif
		#ifdef Q_WS_MAC
		style->setTextDisplacement(1);
		#endif

		QBrush releasedBackground;
		{
			QLinearGradient gradient(0, 0, 0, 1);
			gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
			gradient.setColorAt(0, QColor::fromRgbF(1., 1., 1.));
			gradient.setColorAt(1, QColor::fromRgbF(0.85, 0.85, 0.85));
			releasedBackground = QBrush(gradient);
		}

		style->setBackground(new QxTexture(releasedBackground));

		{
			const char* const grip[] = {
				"7 7 2 1",
				". c None",
				"x c #808080",
				"xxxxxxx",
				".......",
				".......",
				"xxxxxxx",
				".......",
				".......",
				"xxxxxxx"
			};

			style->setMargins(0, 0, 0, 4);
			// style->setRightMargin(4);
			style->setTrailingPatch(QPixmap(grip));
		}

		styleManager()->setConstant("fileBrowserSplitterWidth", 22);
	}
	{
		QxStyle* style = new QxStyle(styleManager()->style("finish"));
		styleManager()->setStyle("fileBrowserToolButton", style);
		style->setMinWidth(style->minHeight() + 4);
	}
	{
		QxStyle* style = new QxStyle(styleManager()->style("fileBrowserToolButton"));
		styleManager()->setStyle("fileBrowserPlusButton", style);

		#include "m0n0/plus.c"
		#include "m0n0/plusNegativ.c"
		QPixmap black; black.loadFromData(plus, plusSize);
		QPixmap white; white.loadFromData(plusNegativ, plusNegativSize);

		QPixmap released(black.width(), black.height() + 1);
		released.fill(Qt::transparent);
		{
			QPainter p(&released);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.7);
			p.drawPixmap(0, 0, black);
		}

		QPixmap pressed(black.width(), black.height() + 1);
		pressed.fill(Qt::transparent);
		{
			QPainter p(&pressed);
			p.drawPixmap(0, 1, white);
			p.drawPixmap(0, 0, black);
		}

		style->setLeadingPatch(released);
		style->setLeadingPatch(pressed, QxControl::Pressed);
		style->setLeadingPatch(pressed, QxControl::Hover);
	}
	{
		QxStyle* style = new QxStyle(styleManager()->style("fileBrowserToolButton"));
		styleManager()->setStyle("fileBrowserWheelButton", style);

		#include "m0n0/wheel.c"
		#include "m0n0/wheelNegativ.c"
		QPixmap black; black.loadFromData(wheel, wheelSize);
		QPixmap white; white.loadFromData(wheelNegativ, wheelNegativSize);

		QPixmap released(black.width(), black.height() + 1);
		released.fill(Qt::transparent);
		{
			QPainter p(&released);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.84);
			p.drawPixmap(0, 0, black);
		}

		QPixmap pressed(black.width(), black.height() + 1);
		pressed.fill(Qt::transparent);
		{
			QPainter p(&pressed);
			p.drawPixmap(0, 1, white);
			p.drawPixmap(0, 0, black);
		}

		style->setTopMargin(1);
		style->setLeadingPatch(released);
		style->setLeadingPatch(pressed, QxControl::Pressed);
		style->setLeadingPatch(pressed, QxControl::Hover);
	}
	{
		QxStyle* style = new QxStyle(styleManager()->style("fileBrowserToolButton"));
		styleManager()->setStyle("fileBrowserRecentButton", style);

		#include "m0n0/history.c"
		#include "m0n0/historyNegativ.c"
		QPixmap black; black.loadFromData(history, historySize);
		QPixmap white; white.loadFromData(historyNegativ, historyNegativSize);

		QPixmap released(black.width(), black.height() + 1);
		released.fill(Qt::transparent);
		{
			QPainter p(&released);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.7);
			p.drawPixmap(0, 0, black);
		}

		QPixmap pressed(black.width(), black.height() + 1);
		pressed.fill(Qt::transparent);
		{
			QPainter p(&pressed);
			p.drawPixmap(0, 1, white);
			p.drawPixmap(0, 0, black);
		}

		style->setTopMargin(1);
		style->setLeadingPatch(released);
		style->setLeadingPatch(pressed, QxControl::Pressed);
		style->setLeadingPatch(pressed, QxControl::Hover);
	}
	{
		QxStyle* style = new QxStyle(styleManager()->style("fileBrowserToolButton"));
		styleManager()->setStyle("fileBrowserBookmarksButton", style);

		#include "m0n0/bookmark.c"
		#include "m0n0/bookmarkNegativ.c"
		QPixmap black; black.loadFromData(bookmark, bookmarkSize);
		QPixmap white; white.loadFromData(bookmarkNegativ, bookmarkNegativSize);

		QPixmap released(black.width(), black.height() + 1);
		released.fill(Qt::transparent);
		{
			QPainter p(&released);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.7);
			p.drawPixmap(0, 0, black);
		}

		QPixmap pressed(black.width(), black.height() + 1);
		pressed.fill(Qt::transparent);
		{
			QPainter p(&pressed);
			p.drawPixmap(0, 1, white);
			p.drawPixmap(0, 0, black);
		}

		style->setTopMargin(1);
		style->setLeadingPatch(released);
		style->setLeadingPatch(pressed, QxControl::Pressed);
		style->setLeadingPatch(pressed, QxControl::Hover);
	}
}

static void codeBrowser()
{
	styleManager()->setStyle("codeBrowserNavBar", styleManager()->style("tabWidgetBar"));
	styleManager()->setStyle("codeBrowserNavCarrier", styleManager()->style("tabWidgetToolHeader"));

	{
		QxStyle* style = new QxStyle(styleManager()->style("codeBrowserNavBar"));
		styleManager()->setStyle("codeBrowserPanelHead", style);
		style->setElideMode(Qt::ElideRight);
		style->setSpacings(5, 5);
		style->setLeftMargin(10);
		style->setRightMargin(10);

		#include "sky/16x16/package.c"
		QPixmap icon; icon.loadFromData(package, packageSize);
		style->setLeadingPatch(icon);
	}

	{
		QxStyle* style = new QxStyle(styleManager()->style("codeBrowserNavBar"));
		styleManager()->setStyle("codeBrowserReloadButton", style);

		style->setRightMargin(5);
		style->setLeftMargin(5);
		// style->setTopMargin(1);

		#include "m0n0/reload.c"
		#include "m0n0/reloadNegativ.c"
		QPixmap black; black.loadFromData(reload, reloadSize);
		QPixmap white; white.loadFromData(reloadNegativ, reloadNegativSize);

		QPixmap released(black.width(), black.height() + 1);
		released.fill(Qt::transparent);
		{
			QPainter p(&released);
			p.drawPixmap(0, 1, white);
			p.setOpacity(0.75);
			p.drawPixmap(0, 0, black);
		}

		QPixmap pressed(black.width(), black.height() + 1);
		pressed.fill(Qt::transparent);
		{
			QPainter p(&pressed);
			p.drawPixmap(0, 1, white);
			p.drawPixmap(0, 0, black);
		}

		style->setLeadingPatch(released);
		style->setLeadingPatch(pressed, QxControl::Pressed);
		style->setLeadingPatch(pressed, QxControl::Hover);
	}

	styleManager()->setColor("codeBrowserInsightViewBgColor", QColor("#E6EDF7"));
}

static void searchEdit()
{
	{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("searchEdit", style);
		style->setDescriptionColor(QColor::fromRgbF(0.5, 0.5, 0.5));
		style->setVirtualText(QString(10, 'n'));
		style->setMargins(6, 8, 6, 12);
		style->setTextAlign(Qt::AlignLeft|Qt::AlignVCenter);
		style->setFontSizePx(12);
		style->setSelectionBackgroundColor(QColor::fromRgbF(0., 0.4, 0.8, 0.25));
		style->setMinWidth(200);
		style->setMaxWidth(200);
		/*#ifdef Q_WS_X11
		style->setTextDisplacement(-1);
		#endif*/
		{
			class Bg: public QxTexture {
			public:
				virtual void draw(QxPainter* p, int x, int y, int w, int h) {
					p->save();
					p->translate(2, 2);
					w -= 5;
					h -= 4;
					p->setRenderHint(QxPainter::Antialiasing, true);
					p->setRenderHint(QxPainter::HighQualityAntialiasing, true);
					p->setPen(QPen(Qt::NoPen));
					qreal s = 1.5;
					qreal r = h / 2.;
					p->setBrush(QColor::fromRgbF(0., 0., 0., 0.3));
					p->drawRoundedRect(0, 0, w - s, h - s, r, r);
					p->setBrush(QColor::fromRgbF(1., 1., 1., 0.4));
					p->drawRoundedRect(0, s, w - s, h - s, r, r);
					p->setBrush(Qt::white);
					p->drawRoundedRect(s, s + 1, w - 2 * s - 1, h - 2 * s - 2, r - s, r - s);
					p->restore();
				}
			};
			class BgInFocus: public Bg {
			public:
				virtual void draw(QxPainter* p, int x, int y, int w, int h) {
					p->save();
					Bg::draw(p, x, y, w, h);
					w -= 5;
					h -= 4;
					p->setRenderHint(QxPainter::Antialiasing, true);
					p->setRenderHint(QxPainter::HighQualityAntialiasing, true);
					p->setPen(QPen(QColor::fromRgbF(0., 0.4, 0.8, 0.25), 4));
					qreal r = h / 2.;
					p->drawRoundedRect(2, 2, w - 1.5, h - 1.5, r, r);
					p->restore();
				}
			};
			style->setBackground(new Bg);
			style->setBackground(new BgInFocus, QxControl::Focus);
		}
		{
			#include "m0n0/magnifierPopup.c"
			QPixmap black;
			black.loadFromData(magnifierPopup, magnifierPopupSize);
			QPixmap blended(black.size());
			blended.fill(Qt::transparent);
			{
				QPainter p(&blended);
				p.setOpacity(0.6);
				p.drawPixmap(0, 0, black);
			}
			style->setLeadingPatch(blended);
			style->setSpacings(4, 0);
		}
	}
	{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("searchEditItem", style);
		style->setBackground(new QxTexture(Qt::white));
		style->setMargins(4, 4, 4, 4);
		style->setBackground(new QxTexture(QColor("#3875d7")), QxControl::Selected);
		style->setMinWidth(216);
		style->setMaxWidth(400);
	}
	{
		QxStyle* style = new QxStyle;
		styleManager()->setStyle("searchEditItemLabel", style);
		style->setTextAlign(Qt::AlignLeft);
		style->setTextColor(Qt::black);
		style->setTextColor(Qt::white, QxControl::Selected);
		style->setElideMode(Qt::ElideRight);
		style->setLeftMargin(4);
	}
	{
		QxStyle* style = new QxStyle(styleManager()->style("searchEditItemLabel"));
		styleManager()->setStyle("searchEditItemDetails", style);
		style->setFontSizePx(12);
		style->setTextColor(Qt::darkBlue);
		style->setTextColor(Qt::white, QxControl::Selected);
		style->setElideMode(Qt::ElideMiddle);
		style->setTopMargin(3);
	}
}

void QxPacuTheme::load()
{
	core();
	tabWidget();
	fileBrowser();
	codeBrowser();
	searchEdit();
}

} // namespace pacu
