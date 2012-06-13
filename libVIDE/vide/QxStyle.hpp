#ifndef VIDE_QXSTYLE_HPP
#define VIDE_QXSTYLE_HPP

#include <QList>
#include <QSet>
#include "UseFtl.hpp"
#include "QxTexture.hpp"
#include "QxPainter.hpp"

namespace vide
{

class QxVisual;

class QxStyle: public QObject, public Instance, public virtual BackRefList
{
	Q_OBJECT
	
public:
	QxStyle(QxStyle* parent = 0);
	~QxStyle();
	
	int textAlign() const;
	void setTextAlign(int flags);
	
	QColor textColor(int state = 0) const;
	void setTextColor(QColor color, int state = 0);
	
	QColor descriptionColor() const;
	void setDescriptionColor(QColor color);
	
	int textDisplacement() const;
	void setTextDisplacement(int value);
	
	QColor selectionBackgroundColor() const;
	void setSelectionBackgroundColor(QColor color);
	
	QString virtualText() const;
	void setVirtualText(QString text);
	
	QFont font() const;
	void setFont(QFont font);
	void setFontSizePx(int size);
	
	Qt::TextElideMode elideMode() const;
	void setElideMode(Qt::TextElideMode mode);
	
	bool engraving() const;
	void setEngraving(bool on);
	
	QColor engravingColor() const;
	void setEngravingColor(QColor color);
	
	int cursorWidth() const;
	void setCursorWidth(int value);
	
	QPixmap leadingPatch(int state = 0) const;
	QPixmap trailingPatch(int state = 0) const;
	void setLeadingPatch(QPixmap pixmap, int state = 0);
	void setTrailingPatch(QPixmap pixmap, int state = 0);
	
	QxTexture* background(int state = 0) const;
	void setBackground(QxTexture* texture, int state = 0);
	
	int topMargin() const;
	int leftMargin() const;
	int bottomMargin() const;
	int rightMargin() const;
	void setTopMargin(int value);
	void setLeftMargin(int value);
	void setBottomMargin(int value);
	void setRightMargin(int value);
	void setMargins(int top, int left, int bottom, int right);
	
	int leadingSpace() const;
	int trailingSpace() const;
	void setLeadingSpace(int value);
	void setTrailingSpace(int value);
	void setSpacings(int leading, int trailing);
	
	int minWidth() const;
	int minHeight() const;
	void setMinWidth(int value);
	void setMinHeight(int value);
	
	int maxWidth() const;
	int maxHeight() const;
	void setMaxWidth(int value);
	void setMaxHeight(int value);
	
private:
	friend class QxVisual;
	
	void registerChild(QxStyle* style);
	void unregisterChild(QxStyle* style);
	
	void registerVisual(QxVisual* visual);
	void unregisterVisual(QxVisual* visual);
	
	void updateGeometry();
	void update();
	
	Ref<QxStyle, Owner> parent_;
	QList< Ref<QxStyle, SetNull> > children_;
	QList< Ref<QxVisual, SetNull> > visuals_;
	
	int textAlign_;
	QVector<QColor> textColor_;
	QColor descriptionColor_;
	int textDisplacement_;
	QColor selectionTextColor_;
	QColor selectionBackgroundColor_;
	QString virtualText_;
	QFont font_;
	Qt::TextElideMode elideMode_;
	bool engraving_;
	QColor engravingColor_;
	int cursorWidth_;
	QVector<QPixmap> leadingPatch_;
	QVector<QPixmap> trailingPatch_;
	QVector< Ref<QxTexture, Owner> > background_;
	int topMargin_;
	int leftMargin_;
	int bottomMargin_;
	int rightMargin_;
	int leadingSpace_;
	int trailingSpace_;
	int minWidth_;
	int minHeight_;
	int maxWidth_;
	int maxHeight_;
	
	QSet<void*> overloaded_;
};

} // namespace vide

#endif // VIDE_QXSTYLE_HPP
