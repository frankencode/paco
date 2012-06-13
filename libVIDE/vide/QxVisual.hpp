#ifndef VIDE_QXVISUAL_HPP
#define VIDE_QXVISUAL_HPP

#include "QxPainter.hpp"
#include "UseFtl.hpp"

namespace vide
{

class QxControl;
class QxStyle;

class QxVisual: public QObject, public Instance, public virtual BackRefList
{
	Q_OBJECT
	
public:
	QxVisual(QxStyle* style = 0);
	~QxVisual();
	
	QxControl* control() const;
	QxStyle* style() const;
	
	QString text() const;
	
	QString description() const;
	void setDescription(QString text);
	
	QPixmap leadingIcon(int state = 0) const;
	QPixmap trailingIcon(int state = 0) const;
	void setLeadingIcon(QPixmap pixmap, int state = 0);
	void setTrailingIcon(QPixmap pixmap, int state = 0);
	
	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;
	virtual QSizePolicy sizePolicy() const;
	virtual void draw(
		QxPainter* p, int width, int height, int state,
		int cx = 0, int cy = 0, int* ti = 0, bool mouseDrag = false
	);
	
	QSize sizeHint(QxStyle* styleOverload);
	
signals:
	void textChanged(QString text);
	
public slots:
	void setText(QString text);
	void resetText();
	
private:
	friend class QxControl;
	
	void drawText(
		QxPainter* p, int x, int y, int w, int h,
		int cx = 0, int cy = 0, int* ti = 0, bool mouseDrag = false,
		QColor color = QColor()
	);
	
	QSize computeSize(bool min) const;
	void updateGeometry();
	
	Ref<QxControl, SetNull> control_;
	Ref<QxStyle, Owner> style_;
	
	QString text_;
	bool hasText_;
	QString description_;
	QVector<QPixmap> leadingIcon_;
	QVector<QPixmap> trailingIcon_;
	
	QStringList lines_;
	QStringList descriptionLines_;
	QRect textRect_;
	QRect leadingRect_;
	QRect trailingRect_;
	QPoint cursorPos_;
	QSize cursorSize_;
	qreal lineHeight_;
};

} // namespace vide

#endif // VIDE_QXVISUAL_HPP
