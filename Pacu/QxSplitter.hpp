#ifndef PACU_QXSPLITTER_HPP
#define PACU_QXSPLITTER_HPP

#include <QMap>
#include <QSplitter>

namespace pacu
{

class QxSplitter: public QSplitter
{
public:
	QxSplitter(QWidget* parent = 0);
	QxSplitter(Qt::Orientation orientation, QWidget* parent = 0);
	
	void setHandle(int index, QWidget* handle);
	virtual QSize minimumSizeHint() const { return QSize(100, 100); }
	
private:
	void paintEvent(QPaintEvent* event) {} // performance HACK
	
	QSplitterHandle* createHandle();
	QMap<int, QWidget*> handles_;
};

} // namespace pacu

#endif // PACU_QXSPLITTER_HPP

