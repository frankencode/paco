#ifndef PACU_QXACTIVATIONDIALOG_HPP
#define PACU_QXACTIVATIONDIALOG_HPP

#include "QxDialog.hpp"

class QLabel;
class QLineEdit;

namespace pacu
{

class QxActivationDialog: public QxDialog
{
	Q_OBJECT
	
public:
	QxActivationDialog(QWidget* parent);
	
private slots:
	void activate();
	void buy();
	void lost();
	
private:
	void updateActivationStatus();
	QLineEdit* licenseHolder_;
	QLabel* activationStatus_;
};

} // namespace pacu

#endif // PACU_QXACTIVATIONDIALOG_HPP
