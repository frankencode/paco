#include <QApplication>
#include "QxPainter.hpp"
#include "QxControl.hpp"
#include "QxVisual.hpp"
#include "QxStyle.hpp"
#include "QxStyleManager.hpp"

namespace vide
{

Ref<QxStyleManager, Owner> QxStyleManager::instance_ = 0;

QxStyleManager* QxStyleManager::instance()
{
	if (!instance_)
		instance_ = new QxStyleManager;
	return instance_;
}

QxStyleManager::QxStyleManager()
{
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(disband()));
}

void QxStyleManager::disband()
{
	instance_ = 0;
}

QxStyle* QxStyleManager::style(QString name) const
{
	return styles_.contains(name) ? styles_.value(name) : 0;
}

void QxStyleManager::setStyle(QString name, QxStyle* style)
{
	if (styles_.contains(name))
		; // not implemented yet, HACK
	else
		styles_.insert(name, style);
}

int QxStyleManager::constant(QString name) const
{
	return constants_.contains(name) ? constants_.value(name) : 0;
}

void QxStyleManager::setConstant(QString name, int value)
{
	if (constants_.contains(name))
		; // not implemented yet, HACK
	else
		constants_.insert(name, value);
}

QColor QxStyleManager::color(QString name) const
{
	return colors_.contains(name) ? colors_.value(name) : QColor();
}

void QxStyleManager::setColor(QString name, QColor value)
{
	if (colors_.contains(name))
		; // not implemented yet, HACK
	else
		colors_.insert(name, value);
}

QxControl* QxStyleManager::hl(QWidget* parent) const
{
	QxControl* control = new QxControl(parent, new QxVisual(style("border")));
	control->setFixedHeight(1);
	return control;
}

QxControl* QxStyleManager::vl(QWidget* parent) const
{
	QxControl* control = new QxControl(parent, new QxVisual(style("border")));
	control->setFixedWidth(1);
	return control;
}

QWidget* QxStyleManager::space(int w, int h, QWidget* parent)
{
	class QxSpacer: public QWidget
	{
	public:
		QxSpacer(int w, int h, QWidget* parent)
			: QWidget(parent)
		{
			QSizePolicy sizePolicy(
				(w == -1) ? QSizePolicy::Expanding : QSizePolicy::Fixed,
				(h == -1) ? QSizePolicy::Expanding : QSizePolicy::Fixed
			);
			
			setSizePolicy(sizePolicy);
			
			if (w != -1) setFixedWidth(w);
			if (h != -1) setFixedHeight(h);
		}
		
		QSize sizeHint() const { return QSize(1, 1); }
	};
	
	return new QxSpacer(w, h, parent);
}

QWidget* QxStyleManager::hFill(QColor fill, int height, QWidget* parent)
{
	class QxFiller: public QWidget
	{
	public:
		QxFiller(QBrush fill, int height, QWidget* parent)
			: QWidget(parent),
			  fill_(fill)
		{
			setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
			setAttribute(Qt::WA_OpaquePaintEvent);
			if (height != -1)
				setFixedHeight(height);
		}
	private:
		void paintEvent(QPaintEvent* event) {
			QxPainter(this).fillRect(0, 0, width(), height(), fill_);
		}
		QBrush fill_;
	};
	
	return new QxFiller(fill, height, parent);
}

QWidget* QxStyleManager::hFill(QColor fill, QWidget* parent) { return hFill(fill, -1, parent); }

QxStyleManager* styleManager() { return QxStyleManager::instance(); }

} // namespace vide
