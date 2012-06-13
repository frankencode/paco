#include <QApplication>
#include "QxControl.hpp"
#include "QxStyle.hpp"
#include "QxVisual.hpp"

namespace vide
{

QxVisual::QxVisual(QxStyle* style)
	: style_((style) ? style : new QxStyle),
	  hasText_(false),
	  leadingIcon_(QxControl::NumState),
	  trailingIcon_(QxControl::NumState),
	  lineHeight_(0)
{
	style_->registerVisual(this);
}

QxVisual::~QxVisual()
{
	style_->unregisterVisual(this);
}

QxControl* QxVisual::control() const { return control_; }
QxStyle* QxVisual::style() const { return style_; }

QString QxVisual::text() const { return text_; }
void QxVisual::setText(QString text) {
	text_ = text;
	hasText_ = true;
	lines_ = text.split('\n');
	//if (control_->hasSelection())
	/*if (control_->mode_ == QxControl::LineEditMode)
		control_->selectAll();
	if ( (control_->mode_ == QxControl::LineEditMode) ||
	     (control_->mode_ == QxControl::TextEditMode) )
		control_->cursorPos_ = text.length();*/
	updateGeometry();
	emit textChanged(text_);
}
void QxVisual::resetText() {
	text_ = QString();
	hasText_ = false;
	lines_.clear();
	/*if (control_->hasSelection())
		control_->unselect();
	control_->cursorPos_ = 0;*/
	updateGeometry();
	emit textChanged(text_);
}

QString QxVisual::description() const { return description_; }
void QxVisual::setDescription(QString text) {
	description_ = text;
	descriptionLines_ = text.split('\n');
	updateGeometry();
}

QPixmap QxVisual::leadingIcon(int state) const { return leadingIcon_[state]; }
QPixmap QxVisual::trailingIcon(int state) const { return trailingIcon_[state]; }
void QxVisual::setLeadingIcon(QPixmap pixmap, int state) {
	leadingIcon_[state] = pixmap;
	updateGeometry();
}
void QxVisual::setTrailingIcon(QPixmap pixmap, int state) {
	trailingIcon_[state] = pixmap;
	updateGeometry();
}

QSize QxVisual::sizeHint() const { return computeSize(false); }
QSize QxVisual::minimumSizeHint() const { return computeSize(true); }
QSizePolicy QxVisual::sizePolicy() const { return QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred); }

QSize QxVisual::sizeHint(QxStyle* styleOverload)
{
	Ref<QxStyle, Owner> styleSaved = style_;
	style_ = styleOverload;
	QSize h = sizeHint();
	style_ = styleSaved;
	return h;
}

void QxVisual::draw(QxPainter* p, int width, int height, int state, int cx, int cy, int* ti, bool mouseDrag)
{
	QColor textColor = style_->textColor_[state];
	QPixmap patch0 = style_->leadingPatch_[state];
	QPixmap patch1 = style_->trailingPatch_[state];
	QxTexture* bg = style_->background_[state];
	QPixmap icon0 = leadingIcon_[state];
	QPixmap icon1 = trailingIcon_[state];
	
	// implemention HACK
	if (((state & QxControl::Focus) == 0) && (descriptionLines_.count() > 0) && (text_.length() == 0))
		textColor = style_->descriptionColor_;
	
	int fallback[] = { QxControl::Hover, QxControl::Inactive, QxControl::Disabled, QxControl::Pressed, QxControl::Focus, QxControl::Selected, -1 };
	for (int i = 0; fallback[i] != -1; ++i) {
		if ((state & fallback[i]) == 0) continue;
		state ^= fallback[i];
		if (!textColor.isValid()) textColor = style_->textColor_[state];
		if (patch0.isNull()) patch0 = style_->leadingPatch_[state];
		if (patch1.isNull()) patch1 = style_->trailingPatch_[state];
		if (!bg) bg = style_->background_[state];
		if (icon0.isNull()) icon0 = leadingIcon_[state];
		if (icon1.isNull()) icon1 = trailingIcon_[state];
	}
	
	QPixmap pm0 = patch0.isNull() ? icon0 : patch0;
	QPixmap pm1 = patch1.isNull() ? icon1 : patch1;
	int mx0 = style_->leftMargin_, mx1 = style_->rightMargin_;
	int my0 = style_->topMargin_, my1 = style_->bottomMargin_;
	int s0 = style_->leadingSpace_, s1 = style_->trailingSpace_;
	int w = width, h = height;
	qreal x0 = 0, y0 = 0;
	
	if ((p) && (bg))
		bg->draw(p, x0, y0, w, h);
	
	w -= (mx0 + mx1);
	h -= (my0 + my1);
	if ((w <= 0) || (h <= 0)) return;
	
	x0 += mx0;
	y0 += my0;
	
	int x = 0, y = 0;
	
	if (!pm0.isNull()) {
		if (!hasText_)
			if (pm0.width() < w)
				x = (w - pm0.width()) / 2;
		if (pm0.height() < h)
			y = (h - pm0.height()) / 2;
		leadingRect_ = QRect(x0 + x, y0 + y, pm0.width(), pm0.height());
		if (p) {
			p->drawPixmap(leadingRect_.x(), leadingRect_.y(), pm0);
			if ((!patch0.isNull()) && (!icon0.isNull()))
				p->drawPixmap(x0 + x + (patch0.width() - icon0.width()) / 2, y0 + y + (patch0.height() - icon0.height()) / 2, icon0);
		}
	}
	
	if (lines_.count() == 0) return;
	
	int dx = x + pm0.width() + s0;
	w -= dx;
	if (w < 0) return;
	x0 += dx;
	
	if (!pm1.isNull()) {
		x = w - pm1.width();
		y = 0;
		if (x <= 0) return;
		if (pm1.height() < h)
			y = (h - pm1.height()) / 2;
		trailingRect_ = QRect(x0 + x, y0 + y, pm1.width(), pm1.height());
		w = x - s1;
		if (p) {
			p->drawPixmap(trailingRect_.x(), trailingRect_.y(), pm1);
			if ((!patch1.isNull()) && (!icon1.isNull()))
				p->drawPixmap(x0 + x + (patch1.width() - icon1.width()) / 2, y0 + y + (patch1.height() - icon1.height()) / 2, icon1);
		}
	}
	
	textRect_ = QRect(x0, y0, w, h);
	drawText(p, x0, y0, w, h, cx, cy, ti, mouseDrag, textColor);
}

void QxVisual::drawText(QxPainter* p, int x, int y, int w, int h, int cx, int cy, int* ti, bool mouseDrag, QColor color)
{
	// implemention HACK
	QStringList lines = lines_;
	if (((control_->state() & QxControl::Focus) == 0) && (descriptionLines_.count() > 0) && (text_.length() == 0))
		lines = descriptionLines_;
	
	QFontMetricsF fm(style_->font_);
	QFontMetrics fmi(style_->font_);
	qreal yt = y, ht = lines.count() * fm.height();
	if ((style_->textAlign_ & Qt::AlignVCenter) != 0)
		yt = y + (h - ht) / 2;
	else if ((style_->textAlign_ & Qt::AlignBottom) != 0)
		yt = y + h + ht;
	if (p) {
		p->save();
		p->setFont(style_->font_);
	}
	if (ti)
		*ti = -1;
	int ti0 = 0;
	int ci = (control_) ? control_->cursorPos() : -1;
	for (int i = 0; i < lines.count(); ++i) {
		QString ln = lines.at(i);
		if (style_->elideMode_ != Qt::ElideNone)
			ln = fmi.elidedText(ln, style_->elideMode_, w + 1); // +1, rounding HACK, exspecially for Cocoa
		if (control_) {
			if (control_->mode_ == QxControl::LineEditMode) {
				while ((fmi.width(ln, ci - ti0) > w) && (ln.length() > 0)) {
					ln.remove(0, 1);
					++ti0;
				}
			}
		}
		if (fmi.width(ln) > w) {
			int j = 0;
			while (j < ln.length()) {
				if (fmi.width(ln, j + 1) > w) break;
				++j;
			}
			ln = ln.left(j);
		}
		qreal xt = x, wt = fm.width(ln);
		if ((style_->textAlign_ & Qt::AlignHCenter) != 0)
			xt = x + (w - wt) / 2;
		else if ((style_->textAlign_ & Qt::AlignRight) != 0)
			xt = x + w - wt;
		if (ti) {
			if ( ((yt <= cy) && (cy < yt + fm.height())) ||
			     (mouseDrag && (((i == 0) && (cy < yt)) || ((i == lines.count() - 1) && (yt + fm.height() <= cy)))) ) {
				*ti = ti0;
				for (int j = 0; j < ln.length(); ++j) {
					if (cx < (xt + fmi.width(ln, j) + fmi.width(ln.at(j)) / 2)) break;
					++*ti;
				}
			}
		}
		if (p) {
			yt += style_->textDisplacement_;
			if (control_) {
				if (control_->hasSelection_) {
					int si0, si1;
					control_->getSelection(&si0, &si1);
					bool outside = (si1 <= ti0) || (ti0 + ln.length() <= si0);
					if (!outside) {
						si0 -= ti0;
						si1 -= ti0;
						if (si0 < 0) si0 = 0;
						if (si1 > ln.length()) si1 = ln.length();
						p->fillRect(xt + fmi.width(ln, si0), yt, fmi.width(ln, si1) - fmi.width(ln, si0), int(fm.ascent() + fm.descent()), style_->selectionBackgroundColor_);
					}
				}
			}
			qreal ytd = 0;
			if (style_->engraving_) {
				p->setPen(style_->engravingColor_);
				p->drawText(xt, yt + 0.5 + fm.ascent(), ln);
				ytd = -0.5;
			}
			if (!color.isValid()) color = Qt::black;
			p->setPen(color);
			p->drawText(xt, yt + ytd + fm.ascent(), ln);
			yt -= style_->textDisplacement_;
			if (control_) {
				if ((ti0 <= ci) && (ci <= ti0 + ln.length())) {
					int cx = fmi.width(ln, ci - ti0);
					cursorPos_ = QPoint(xt + cx, yt + ytd);
					cursorSize_ = QSize(style_->cursorWidth_, fmi.ascent() + fmi.descent());
					lineHeight_ = fm.height();
					if (control_->blink_)
						p->fillRect(cursorPos_.x(), cursorPos_.y(), cursorSize_.width(), cursorSize_.height(), color);
				}
			}
		}
		ti0 += ln.length() + 1;
		yt += fm.height();
	}
	if (p)
		p->restore();
}

QSize QxVisual::computeSize(bool min) const
{
	int wp0 = 0, hp0 = 0;
	int wp1 = 0, hp1 = 0;
	for (int state = 0; state < QxControl::NumState; ++state) {
		QPixmap pm0 = style_->leadingPatch_[state].isNull() ? leadingIcon_[state] : style_->leadingPatch_[state];
		QPixmap pm1 = style_->trailingPatch_[state].isNull() ? trailingIcon_[state] : style_->trailingPatch_[state];
		if (!pm0.isNull()) {
			if (pm0.width() > wp0)
				wp0 = pm0.width();
			if (pm0.height() > hp0)
				hp0 = pm0.height();
		}
		if (!pm1.isNull()) {
			if (pm1.width() > wp1)
				wp1 = pm1.width();
			if (pm1.height() > hp1)
				hp1 = pm1.height();
		}
	}
	
	QFontMetrics fm(style_->font_);
	
	int w = 0;
	w += style_->leftMargin_;
	w += wp0;
	if (hasText_) {
		w += style_->leadingSpace_ + style_->trailingSpace_;
		bool notLineEditMode = (control_) ? (control_->mode_ != QxControl::LineEditMode) : true;
		if (notLineEditMode) {
			if (min || (style_->elideMode_ == Qt::ElideNone)) {
				int wt = 0;
				for (int i = 0; i < lines_.count(); ++i) {
					int wl = fm.width(lines_.at(i));
					if (wl > wt) wt = wl;
				}
				w += wt;
			}
		}
		if (style_->virtualText_ != QString()) {
			if (!text_.contains(style_->virtualText_))
				w += fm.width(style_->virtualText_);
		}
	}
	w += wp1;
	w += style_->rightMargin_;
	
	int h = 0, hc = 0;
	h += style_->topMargin_;
	if (hasText_)
		hc = (lines_.count() > 0) ? fm.height() * lines_.count() : fm.height();
	
	if (hp0 > hc)
		hc = hp0;
	if (hp1 > hc)
		hc = hp1;
	h += hc;
	h += style_->bottomMargin_;
	
	if (w < style_->minWidth_)
		w = style_->minWidth_;
	if (h < style_->minHeight_)
		h = style_->minHeight_;
	
	return QSize(w, h);
}

void QxVisual::updateGeometry()
{
	if (control_) {
		control_->updateGeometry();
		control_->update();
	}
}

} // namespace vide
