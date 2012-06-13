#ifndef PACU_QXSPLITTERHANDLE_HPP
#define PACU_QXSPLITTERHANDLE_HPP

#include <QSplitterHandle>

namespace pacu
{

class QxSplitterHandle: public QSplitterHandle
{
public:
	QxSplitterHandle(Qt::Orientation orientation, QSplitter* parent = 0, int index = 0, QWidget* bar = 0);
	
protected:
	bool collapsed() const;
	
private:
	int index_;
};

} // namespace pacu

#endif // PACU_QXSPLITTERHANDLE_HPP
