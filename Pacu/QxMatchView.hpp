#ifndef PACU_QXMATCHVIEW_HPP
#define PACU_QXMATCHVIEW_HPP

#include <QMouseEvent>
#include <pte/View.hpp>
#include "UsePte.hpp"

namespace pacu
{

class QxMatchView: public View
{
	Q_OBJECT
	
public:
	QxMatchView(QWidget* parent = 0);
	
public slots:
	void openLink();
	void firstLink();
	void lastLink();
	void previousLink();
	void nextLink();
	
signals:
	void gotoMatch(Ref<Document>, int my, int mx0, int mx1);
	
private:
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
};

} // namespace pacu

#endif // PACU_QXMATCHVIEW_HPP
