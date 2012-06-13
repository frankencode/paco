#ifndef PACU_QXURLLISTVIEW_HPP
#define PACU_QXURLLISTVIEW_HPP

#include <QListView>

namespace pacu
{

class QxUrlListView: public QListView
{
	Q_OBJECT
	
public:
	QxUrlListView(QWidget* parent = 0);
	
	// virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
	
private:
	virtual void focusOutEvent(QFocusEvent* event);
};

} // namespace pacu

#endif // PACU_QXURLLISTVIEW_HPP
