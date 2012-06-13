#include "QxTestPalette.hpp"

namespace pacu
{

QxTestPalette::QxTestPalette(QPalette::ColorGroup colorGroup, QWidget* parent)
	: QWidget(parent),
	  colorGroup_(colorGroup)
{
	if (colorGroup == QPalette::Disabled)
		setWindowTitle("QPalette::Disabled");
	else if (colorGroup == QPalette::Active)
		setWindowTitle("QPalette::Active");
	else
		setWindowTitle("QPalette::Inactive");
}

void QxTestPalette::paintEvent(QPaintEvent* event)
{
	struct { const char* name; QPalette::ColorRole value; } role[] = {
		{"QPalette::Window", QPalette::Window},
		{"QPalette::Background", QPalette::Background},
		{"QPalette::WindowText", QPalette::WindowText},
		{"QPalette::Foreground", QPalette::Foreground},
		{"QPalette::Base", QPalette::Base},
		{"QPalette::AlternateBase", QPalette::AlternateBase},
		// {"QPalette::ToolTipBase", QPalette::ToolTipBase},
		{"QPalette::Text", QPalette::Text},
		
		{"QPalette::Button", QPalette::Button},
		{"QPalette::ButtonText", QPalette::ButtonText},
		{"QPalette::BrightText", QPalette::ButtonText},
		
		{"QPalette::Light", QPalette::Light},
		{"QPalette::Midlight", QPalette::Midlight},
		{"QPalette::Dark", QPalette::Dark},
		{"QPalette::Mid", QPalette::Mid},
		{"QPalette::Shadow", QPalette::Shadow},
		
		{"QPalette::Highlight", QPalette::Highlight},
		{"QPalette::HighlightedText", QPalette::HighlightedText},
		
		{"QPalette::Link", QPalette::Link},
		{"QPalette::LinkVisited", QPalette::LinkVisited}
	};
	
	const int numRoles = sizeof(role) / sizeof(role[0]);
	
	QPainter p(this);
	for (int i = 0; i < numRoles; ++i)
	{
		qreal h = qreal(height()) / numRoles;
		QRectF rect(0, i * h, width(), h);
		p.setBrush(palette().brush(colorGroup_, role[i].value));
		p.drawRect(rect);
		p.setPen(Qt::white);
		p.drawText(rect, Qt::AlignLeft, role[i].name);
		p.setPen(Qt::black);
		p.drawText(rect, Qt::AlignRight, role[i].name);
	}
}

} // namespace pacu
