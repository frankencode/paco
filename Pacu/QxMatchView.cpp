#include <QDebug>
#include "QxMatchView.hpp"

namespace pacu
{

QxMatchView::QxMatchView(QWidget* parent)
	: View(parent)
{
	setCursor(Qt::ArrowCursor);
	setMouseTracking(true);
	setLeadingMargin(2);
}

void QxMatchView::openLink()
{
	Ref<MatchStatus> matchStatus = document()->matchStatus();
	if (matchStatus && hasSelection()) {
		int selya, selxa, selyb, selxb;
		getSelection(&selya, &selxa, &selyb, &selxb);
		Ref<MatchingLine> ml = matchStatus->matchingLines_->get(selya);
		emit gotoMatch(ml->document_, ml->y_, selxa, selxb);
	}
}

void QxMatchView::firstLink()
{
	Ref<MatchStatus> matchStatus = document()->matchStatus();
	if (matchStatus) {
		Ref<MatchingLine> ml;
		Ref<MatchList> matches;
		int my = -1;
		for (int y = 0, n = cache()->length(); (y < n) && (!matches); ++y) {
			ml = matchStatus->matchingLines_->get(y);
			matches = ml->matches_;
			my = y;
		}
		if (matches) {
			Match match = matches->get(0);
			emit gotoMatch(ml->document_, ml->y_, match.x0_, match.x1_);
			select(my, match.x0_, my, match.x1_);
			keepInView(my, match.x0_);
			update();
		}
	}
}

void QxMatchView::lastLink()
{
	Ref<MatchStatus> matchStatus = document()->matchStatus();
	if (matchStatus) {
		Ref<MatchingLine> ml;
		Ref<MatchList> matches;
		int my = -1;
		for (int y = cache()->length() - 1; (y >= 0) && (!matches); ++y) {
			ml = matchStatus->matchingLines_->get(y);
			matches = ml->matches_;
			my = y;
		}
		if (matches) {
			Match match = matches->get(matches->length() - 1);
			emit gotoMatch(ml->document_, ml->y_, match.x0_, match.x1_);
			select(my, match.x0_, my, match.x1_);
			keepInView(my, match.x0_);
			update();
		}
	}
}

void QxMatchView::previousLink()
{
	if (!hasSelection()) {
		lastLink();
		return;
	}
	Ref<MatchStatus> matchStatus = document()->matchStatus();
	if (matchStatus && hasSelection()) {
		int selya, selxa, selyb, selxb;
		getSelection(&selya, &selxa, &selyb, &selxb);
		Ref<MatchingLine> ml = matchStatus->matchingLines_->get(selya);
		Ref<MatchList> matches = ml->matches_;
		for (int n = matches->length(), i = n - 1; i >= 0; --i) {
			Match match = matches->get(i);
			if (match.x1_ <= selxa) {
				emit gotoMatch(ml->document_, ml->y_, match.x0_, match.x1_);
				select(selya, match.x0_, selya, match.x1_);
				keepInView(selya, match.x0_);
				update();
				return;
			}
		}
		matches = 0;
		int my = -1;
		for (int y = selya - 1; (y >= 0) && (!matches); --y) {
			ml = matchStatus->matchingLines_->get(y);
			matches = ml->matches_;
			my = y;
		}
		if (matches) {
			Match match = matches->get(matches->length() - 1);
			emit gotoMatch(ml->document_, ml->y_, match.x0_, match.x1_);
			select(my, match.x0_, my, match.x1_);
			keepInView(my, match.x0_);
			update();
		}
	}
}

void QxMatchView::nextLink()
{
	if (!hasSelection()) {
		firstLink();
		return;
	}
	Ref<MatchStatus> matchStatus = document()->matchStatus();
	if (matchStatus) {
		int selya, selxa, selyb, selxb;
		getSelection(&selya, &selxa, &selyb, &selxb);
		Ref<MatchingLine> ml = matchStatus->matchingLines_->get(selya);
		Ref<MatchList> matches = ml->matches_;
		for (int i = 0, n = matches->length(); i < n; ++i) {
			Match match = matches->get(i);
			if (selxb <= match.x0_) {
				emit gotoMatch(ml->document_, ml->y_, match.x0_, match.x1_);
				select(selya, match.x0_, selya, match.x1_);
				keepInView(selya, match.x0_);
				update();
				return;
			}
		}
		matches = 0;
		int my = -1;
		for (int y = selya + 1, n = cache()->length(); (y < n) && (!matches); ++y) {
			ml = matchStatus->matchingLines_->get(y);
			matches = ml->matches_;
			my = y;
		}
		if (matches) {
			Match match = matches->get(0);
			emit gotoMatch(ml->document_, ml->y_, match.x0_, match.x1_);
			select(my, match.x0_, my, match.x1_);
			keepInView(my, match.x0_);
			update();
		}
	}
}

void QxMatchView::mousePressEvent(QMouseEvent* event)
{
	int ty, tx;
	mapMouseToTextPos(event->x() + glyphWidth() / 2, event->y(), &ty, &tx);
	if ((ty == -1) || (tx == -1)) return;
	Ref<MatchList> matches = document()->matchStatus()->matchingLines_->get(ty)->matches_;
	if (matches) {
		View::mousePressEvent(event);
	}
	else {
		Ref<MatchingLine> ml = document()->matchStatus()->matchingLines_->get(ty);
		emit gotoMatch(ml->document_, -1, -1, -1);
	}
}

void QxMatchView::mouseReleaseEvent(QMouseEvent* event)
{
	if (hasSelection()) return;
	int ty, tx, cy, cx;
	int my = -1, mx0 = -1, mx1 = -1;
	mapMouseToTextPos(event->x() + glyphWidth() / 2, event->y(), &ty, &tx);
	if ((ty == -1) || (tx == -1)) return;
	cy = ty; cx = tx;
	Ref<MatchList> matches = document()->matchStatus()->matchingLines_->get(ty)->matches_;
	if (!matches) return;
	int dm = intMax; // minimum distance
	for (int i = 0, n = matches->length(); i < n; ++i) {
		Match match = matches->get(i);
		int d0 = match.x0_ - tx;
		int d1 = match.x1_ - tx;
		if (d0 < 0) d0 = -d0;
		if (d1 < 0) d1 = -d1;
		if ((d0 < dm) || (d1 < dm)) {
			dm = (d0 < d1) ? d0 : d1;
			my = cy;
			mx0 = match.x0_;
			mx1 = match.x1_;
			cx = mx1;
		}
	}
	if (my != -1) {
		Ref<MatchingLine> ml = document()->matchStatus()->matchingLines_->get(my);
		emit gotoMatch(ml->document_, ml->y_, mx0, mx1);
		select(my, mx0, my, mx1);
	}
	update();
}

void QxMatchView::mouseMoveEvent(QMouseEvent* event)
{
	int ty = -1, tx = -1;
	mapMouseToTextPos(event->x(), event->y(), &ty, &tx);
	if ((ty == -1) || (tx == -1)) return;
	Ref<MatchList> matches = document()->matchStatus()->matchingLines_->get(ty)->matches_;
	if (matches) {
		for (int i = 0, n = matches->length(); i < n; ++i) {
			Match match = matches->get(i);
			if ((match.x0_ <= tx) && (tx < match.x1_)) {
				if (cursor().shape() != Qt::PointingHandCursor) {
					setCursor(Qt::PointingHandCursor);
				}
				return;
			}
		}
	}
	/*else {
		if (cursor().shape() != Qt::PointingHandCursor)
			setCursor(Qt::PointingHandCursor);
		return;
	}*/
	if (cursor().shape() != Qt::ArrowCursor)
		setCursor(Qt::ArrowCursor);
	// View::mouseMoveEvent(event);
}

} // namespace pacu
