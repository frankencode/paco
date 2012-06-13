#ifndef PACU_QXABOUTDIALOG_HPP
#define PACU_QXABOUTDIALOG_HPP

#include "QxDialog.hpp"

namespace pacu
{

class QxAboutDialog: public QxDialog
{
	Q_OBJECT
	
public:
	QxAboutDialog(QWidget* parent);
	
private slots:
	void openUrl(const QString& url);
};

} // namespace pacu

#endif // PACU_QXABOUTDIALOG_HPP
