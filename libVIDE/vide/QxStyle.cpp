#include <QApplication>
#include "QxControl.hpp"
#include "QxVisual.hpp"
#include "QxStyle.hpp"

namespace vide
{

QxStyle::QxStyle(QxStyle* parent)
	: parent_(parent),
	  textAlign_((parent) ? parent->textAlign_ : Qt::AlignCenter),
	  textColor_(QxControl::NumState),
	  textDisplacement_((parent) ? parent->textDisplacement_ : 0),
	  selectionTextColor_((parent) ? parent->selectionTextColor_ : Qt::black),
	  selectionBackgroundColor_((parent) ? parent->selectionBackgroundColor_ : QColor::fromRgbF(0., 0.4, 0.8, 0.25)),
	  virtualText_((parent) ? parent->virtualText_ : QString()),
	  font_((parent) ? parent->font_ : qApp->font()),
	  elideMode_((parent) ? parent->elideMode_ : Qt::ElideNone),
	  engraving_((parent) ? parent->engraving_ : false),
	  engravingColor_((parent) ? parent->engravingColor_ : Qt::lightGray),
	  cursorWidth_(1),
	  leadingPatch_(QxControl::NumState),
	  trailingPatch_(QxControl::NumState),
	  background_(QxControl::NumState),
	  topMargin_((parent) ? parent->topMargin_ : 0),
	  leftMargin_((parent) ? parent->leftMargin_ : 0),
	  bottomMargin_((parent) ? parent->bottomMargin_ : 0),
	  rightMargin_((parent) ? parent->rightMargin_ : 0),
	  leadingSpace_((parent) ? parent->leadingSpace_ : 0),
	  trailingSpace_((parent) ? parent->trailingSpace_ : 0),
	  minWidth_((parent) ? parent->minWidth_ : 0),
	  minHeight_((parent) ? parent->minHeight_ : 0),
	  maxWidth_((parent) ? parent->maxWidth_ : 0),
	  maxHeight_((parent) ? parent->maxHeight_ : 0)
{
	if (parent_) {
		Q_ASSERT(parent_ != this);
		textColor_ = parent_->textColor_;
		leadingPatch_ = parent_->leadingPatch_;
		trailingPatch_ = parent_->trailingPatch_;
		background_ = parent_->background_;
		parent_->registerChild(this);
	}
}

QxStyle::~QxStyle()
{
	if (parent_)
		parent_->unregisterChild(this);
}

#define QXSTYLE_PROPAGATE(attr_, setAttr, value, updateFunc) \
	overloaded_.insert(&attr_); \
	if (attr_ != value) { \
		foreach(QxStyle* child, children_) \
			if (!child->overloaded_.contains(&child->attr_)) { \
				child->attr_ = value; \
				child->updateFunc(); \
			} \
		attr_ = value; \
		updateFunc(); \
	}

// performance HACK, propagation is not appropiatly canceled (because QPixmap is not comparable!)
#define QXSTYLE_PROPAGATE2(attr_, setAttr, value, state, updateFunc) \
	overloaded_.insert(&attr_[state]); \
	foreach(QxStyle* child, children_) \
		if (!child->overloaded_.contains(&child->attr_[state])) { \
			child->setAttr(value, state); \
			child->updateFunc(); \
		} \
	attr_[state] = value; \
	updateFunc(); \

int QxStyle::textAlign() const { return textAlign_; }
void QxStyle::setTextAlign(int flags) {
	QXSTYLE_PROPAGATE(textAlign_, setTextAlign, flags, update);
}

QColor QxStyle::textColor(int state) const { return textColor_[state]; }
void QxStyle::setTextColor(QColor color, int state) {
	QXSTYLE_PROPAGATE2(textColor_, setTextColor, color, state, update);
}

QColor QxStyle::descriptionColor() const { return descriptionColor_; }
void QxStyle::setDescriptionColor(QColor color) {
	QXSTYLE_PROPAGATE(descriptionColor_, setDescriptionColor, color, update);
}

int QxStyle::textDisplacement() const { return textDisplacement_; }
void QxStyle::setTextDisplacement(int value) {
	QXSTYLE_PROPAGATE(textDisplacement_, setTextDisplacement, value, updateGeometry);
}

QColor QxStyle::selectionBackgroundColor() const { return selectionBackgroundColor_; }
void QxStyle::setSelectionBackgroundColor(QColor color) {
	QXSTYLE_PROPAGATE(selectionBackgroundColor_, setSelectionBackgroundColor, color, update);
}

QString QxStyle::virtualText() const { return virtualText_; }
void QxStyle::setVirtualText(QString text) {
	QXSTYLE_PROPAGATE(virtualText_, setVirtualText, text, updateGeometry);
}

QFont QxStyle::font() const { return font_; }
void QxStyle::setFont(QFont font) {
	QXSTYLE_PROPAGATE(font_, setFont, font, updateGeometry);
}
void QxStyle::setFontSizePx(int size) {
	QFont font = font_;
	font.setPixelSize(size);
	QXSTYLE_PROPAGATE(font_, setFont, font, updateGeometry);
}

Qt::TextElideMode QxStyle::elideMode() const { return elideMode_; }
void QxStyle::setElideMode(Qt::TextElideMode mode) {
	QXSTYLE_PROPAGATE(elideMode_, setElideMode, mode, updateGeometry);
}

bool QxStyle::engraving() const { return engraving_; }
void QxStyle::setEngraving(bool on) {
	QXSTYLE_PROPAGATE(engraving_, setEngraving, on, update);
}

QColor QxStyle::engravingColor() const { return engravingColor_; }
void QxStyle::setEngravingColor(QColor color) {
	QXSTYLE_PROPAGATE(engravingColor_, setEngravingColor, color, update);
}

int QxStyle::cursorWidth() const { return cursorWidth_; }
void QxStyle::setCursorWidth(int value) {
	QXSTYLE_PROPAGATE(cursorWidth_, setCursorWidth, value, update);
}

QPixmap QxStyle::leadingPatch(int state) const { return leadingPatch_[state]; }
QPixmap QxStyle::trailingPatch(int state) const { return trailingPatch_[state]; }
void QxStyle::setLeadingPatch(QPixmap pixmap, int state) {
	QXSTYLE_PROPAGATE2(leadingPatch_, setLeadingPatch, pixmap, state, updateGeometry);
}
void QxStyle::setTrailingPatch(QPixmap pixmap, int state) {
	QXSTYLE_PROPAGATE2(trailingPatch_, setTrailingPatch, pixmap, state, updateGeometry);
}

QxTexture* QxStyle::background(int state) const { return background_[state]; }
void QxStyle::setBackground(QxTexture* texture, int state) {
	QXSTYLE_PROPAGATE2(background_, setBackground, texture, state, update);
}

int QxStyle::topMargin() const { return topMargin_; }
int QxStyle::leftMargin() const { return leftMargin_; }
int QxStyle::bottomMargin() const { return bottomMargin_; }
int QxStyle::rightMargin() const { return rightMargin_; }
void QxStyle::setTopMargin(int value) {
	QXSTYLE_PROPAGATE(topMargin_, setTopMargin, value, updateGeometry);
}
void QxStyle::setLeftMargin(int value) {
	QXSTYLE_PROPAGATE(leftMargin_, setLeftMargin, value, updateGeometry);
}
void QxStyle::setBottomMargin(int value) {
	QXSTYLE_PROPAGATE(bottomMargin_, setBottomMargin, value, updateGeometry);
}
void QxStyle::setRightMargin(int value) {
	QXSTYLE_PROPAGATE(rightMargin_, setRightMargin, value, updateGeometry);
}
void QxStyle::setMargins(int top, int left, int bottom, int right) {
	QXSTYLE_PROPAGATE(topMargin_, setTopMargin, top, updateGeometry);
	QXSTYLE_PROPAGATE(leftMargin_, setLeftMargin, left, updateGeometry);
	QXSTYLE_PROPAGATE(bottomMargin_, setBottomMargin, bottom, updateGeometry);
	QXSTYLE_PROPAGATE(rightMargin_, setRightMargin, right, updateGeometry);
}

int QxStyle::leadingSpace() const { return leadingSpace_; }
int QxStyle::trailingSpace() const { return trailingSpace_; }
void QxStyle::setLeadingSpace(int value) {
	QXSTYLE_PROPAGATE(leadingSpace_, setLeadingSpace, value, updateGeometry);
}
void QxStyle::setTrailingSpace(int value) {
	QXSTYLE_PROPAGATE(trailingSpace_, setTrailingSpace, value, updateGeometry);
}
void QxStyle::setSpacings(int leading, int trailing) {
	QXSTYLE_PROPAGATE(leadingSpace_, setLeadingSpace, leading, updateGeometry);
	QXSTYLE_PROPAGATE(trailingSpace_, setTrailingSpace, trailing, updateGeometry);
}

int QxStyle::minWidth() const { return minWidth_; }
int QxStyle::minHeight() const { return minHeight_; }
void QxStyle::setMinWidth(int value) {
	QXSTYLE_PROPAGATE(minWidth_, setMinWidth, value, updateGeometry);
}
void QxStyle::setMinHeight(int value) {
	QXSTYLE_PROPAGATE(minHeight_, setMinHeight, value, updateGeometry);
}

int QxStyle::maxWidth() const { return maxWidth_; }
int QxStyle::maxHeight() const { return maxHeight_; }
void QxStyle::setMaxWidth(int value) {
	QXSTYLE_PROPAGATE(maxWidth_, setMaxWidth, value, updateGeometry);
}
void QxStyle::setMaxHeight(int value) {
	QXSTYLE_PROPAGATE(maxHeight_, setMaxHeight, value, updateGeometry);
}

void QxStyle::registerChild(QxStyle* style)
{
	if (!children_.contains(style))
		children_.append(style);
}

void QxStyle::unregisterChild(QxStyle* style)
{
	children_.removeAll(style);
}

void QxStyle::registerVisual(QxVisual* visual)
{
	if (!visuals_.contains(visual))
		visuals_.append(visual);
}

void QxStyle::unregisterVisual(QxVisual* visual)
{
	visuals_.removeAll(visual);
}

void QxStyle::updateGeometry()
{
	for (int i = 0, n = visuals_.count(); i < n; ++i) {
		QxControl* control = visuals_.at(i)->control();
		if (control) {
			control->updateGeometry();
			if (control->updatesEnabled()) // performance HACK
				control->update();
		}
	}
}

void QxStyle::update()
{
	for (int i = 0, n = visuals_.count(); i < n; ++i) {
		QxControl* control = visuals_.at(i)->control();
		if (control) {
			if (control->updatesEnabled()) // performance HACK
				visuals_.at(i)->control()->update();
		}
	}
}

} // namespace vide
