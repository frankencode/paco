#ifndef PACU_QXCURRENTDIRDIALOG_HPP
#define PACU_QXCURRENTDIRDIALOG_HPP

#include "QxDialog.hpp"

class QToolButton;
class QLineEdit;

namespace pacu
{

class QxCurrentDirDialog: public QxDialog
{
	Q_OBJECT
	
public:
	QxCurrentDirDialog(QWidget* parent);
	QString exec();
	
private slots:
	void cascade();
	
private:
	QToolButton* button_;
	QLineEdit* input_;
};

} // namespace pacu

#endif // PACU_QXCURRENTDIRDIALOG_HPP
