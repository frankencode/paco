#ifndef PACU_QXDIALOG_HPP
#define PACU_QXDIALOG_HPP

#include <QDialog>
#include <QKeyEvent>

namespace pacu
{

class QxDialog: public QDialog
{
public:
	QxDialog(QWidget* parent);
	
private:
	void keyPressEvent(QKeyEvent* event);
};

} // namespace pacu

#endif // PACU_QXDIALOG_HPP

