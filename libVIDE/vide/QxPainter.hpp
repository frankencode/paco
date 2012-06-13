#ifndef VIDE_QXPAINTER_HPP
#define VIDE_QXPAINTER_HPP

#include <QPainter>
#include <QWidget>

namespace vide
{

class QxPainter: public QPainter
{
public:
	QxPainter(QWidget* widget): QPainter(widget), widget_(widget) {}
	
	inline QStyle* style() const { return widget_->style(); }
	inline QWidget* widget() const { return widget_; }
	
private:
	QWidget* widget_;
};

} // namespace vide

#endif // VIDE_QXPAINTER_HPP
