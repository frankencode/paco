#include <QDebug> // DEBUG
#include <QLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <stdlib.h> // free
#include "QxActivation.hpp"
#include "QxActivationDialog.hpp"

namespace pacu
{

using namespace seal;

QxActivationDialog::QxActivationDialog(QWidget* parent)
	: QxDialog(parent)
{
	setWindowTitle(tr("Activation"));
	
	QVBoxLayout* col = new QVBoxLayout;
	{
		QLabel* label = new QLabel;
		label->setTextFormat(Qt::RichText);
		label->setWordWrap(true);
		activationStatus_ = label;
		updateActivationStatus();
		col->addWidget(label);
	}
	{
		QFrame* frame = new QFrame;
		frame->setFrameStyle(QFrame::HLine);
		col->addWidget(frame);
	}
	{
		QGridLayout* grid = new QGridLayout;
		grid->addWidget(new QLabel(tr("License Holder")), 0, 0);
		grid->addWidget(new QLabel(tr("License Key")), 1, 0);
		{
			QLineEdit* edit = new QLineEdit;
			edit->setText(activation()->licenseHolder());
			grid->addWidget(edit, 0, 1);
			connect(edit, SIGNAL(textChanged(const QString&)), activation(), SLOT(setLicenseHolder(const QString&)));
			licenseHolder_ = edit;
		}
		{
			QLineEdit* edit = new QLineEdit(this);
			edit->setText(activation()->licenseKey());
			grid->addWidget(edit, 1, 1);
			connect(edit, SIGNAL(textChanged(const QString&)), activation(), SLOT(setLicenseKey(const QString&)));
		}
		col->addLayout(grid);
	}
	{
		QHBoxLayout* row = new QHBoxLayout;
		QPushButton* activate = new QPushButton;
		activate->setText(tr("Activate"));
		connect(activate, SIGNAL(pressed()), this, SLOT(activate()));
		row->addStretch();
		row->addWidget(activate);
		row->addStretch();
		col->addLayout(row);
	}
	{
		QFrame* frame = new QFrame;
		frame->setFrameStyle(QFrame::HLine);
		col->addWidget(frame);
	}
	{
		QLabel* label =
			new QLabel(
				tr(
					"A single license key allows to activate Pacu on 4 different machines."
					/*"For activation your computer needs to be connected to the Internet.<br/>"
					"After successful activation you can savely disconnect from the Internet.</br>"
					"Only a minimum of information identifying your machine will be transmitted to "
					"the activation server."*/
				)
			);
		/*label->setTextFormat(Qt::RichText);
		label->setWordWrap(true);*/
		col->addWidget(label);
	}
	{
		QPushButton* buy = new QPushButton;
		buy->setText(tr("Buy a new license key"));
		connect(buy, SIGNAL(pressed()), this, SLOT(buy()));
		// col->addWidget(buy, 0, Qt::AlignCenter);
		
		QPushButton* lost = new QPushButton;
		lost->setText(tr("Retrieve lost key"));
		connect(lost, SIGNAL(pressed()), this, SLOT(lost()));
		//col->addWidget(buy, 0, Qt::AlignCenter);
		
		QHBoxLayout* row = new QHBoxLayout;
		row->addWidget(lost);
		row->addStretch();
		row->addWidget(buy);
		col->addLayout(row);
	}
	setLayout(col);
}

void QxActivationDialog::activate()
{
	int error = Activation::NoError;
	char* realLicenseHolder = 0;
	#ifdef Q_WS_X11
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	#endif
	
	QProgressDialog progress(this);
	progress.setMinimumDuration(1000);
	progress.setWindowModality(Qt::WindowModal);
	progress.setLabelText(tr("Connecting to activation server..."));
	progress.setRange(0, 300);
	progress.setValue(0);
	
	bool ok = activation()->verifyLicenseKey(&error, &realLicenseHolder, &progress);
	
	#ifdef Q_WS_X11
	QApplication::restoreOverrideCursor();
	#endif
	if (ok) {
		if (error == Activation::RedundantActivation)
			QMessageBox::information(
				this, qApp->applicationName(),
				tr("%1 has been fully reactivated.").arg(qApp->applicationName())
			);
		else
			QMessageBox::information(
				this, qApp->applicationName(),
				tr("%1 is now fully activated.").arg(qApp->applicationName())
			);
		
		if (realLicenseHolder) {
			licenseHolder_->setText(realLicenseHolder);
			::free(realLicenseHolder);
		}
		accept();
	}
	else if (!progress.wasCanceled()) {
		if (progress.value() == progress.maximum()) {
			QMessageBox::critical(
				this, qApp->applicationName(),
				tr( "Timeout while connecting to activation server.\n"
				    "Please ensure you computer is connected to the internet "
				    "and your firewall is not blocking port 443 (SSL)." )
			);
		}
		else if (error == Activation::TransmissionError) {
			QMessageBox::critical(
				this, qApp->applicationName(),
				tr( "Failed to connect to activation server.\n"
				    "Please ensure you computer is connected to the internet "
				    "and your firewall is not blocking port 443 (SSL)." )
			);
		}
		else if (error == Activation::AnnualActivationLimit) {
			QMessageBox::critical(
				this, qApp->applicationName(),
				tr("Activation failed: Annual machine limit reached.")
			);
		}
		else if (error == Activation::NetworkInterfaceLimit) {
			QMessageBox::critical(
				this, qApp->applicationName(),
				tr("Activation failed: NIC count exhausted.")
			);
		}
		else if (error == Activation::UnknownLicenseKey) {
			QMessageBox::critical(
				this, qApp->applicationName(),
				tr("Activation failed: Unknown license key.")
			);
		}
		else if (error == Activation::InvalidLicenseKey) {
			QMessageBox::critical(
				this, qApp->applicationName(),
				tr("Activation failed: Invalid license key.")
			);
		}
		else if (error == Activation::InvalidRequest) {
			QMessageBox::critical(
				this, qApp->applicationName(),
				tr("Activation failed: Invalid activation request.")
			);
		}
		else if (error == Activation::OutOfMemory) {
			QMessageBox::critical(
				this, qApp->applicationName(),
				tr("Activation failed: Out of memory.")
			);
		}
		else if (error == Activation::InternalError) {
			QMessageBox::critical(
				this, qApp->applicationName(),
				tr("Activation failed: Internal error.")
			);
		}
		else {
			QMessageBox::critical(
				this, qApp->applicationName(),
				tr("Activation failed for unknown reason.")
			);
		}
	}
	
	updateActivationStatus();
}

void QxActivationDialog::buy()
{
	QDesktopServices::openUrl(QUrl(tr("http://www.cyblogic.com/en/store")));
}

void QxActivationDialog::lost()
{
	QDesktopServices::openUrl(QUrl(tr("http://www.cyblogic.com/en/support")));
}

void QxActivationDialog::updateActivationStatus()
{
	QLabel* label = activationStatus_;
	int timeLeft = activation()->timeLeft();
	if (timeLeft <= 0) {
		label->setText(
			tr( "The evaluation period has been <b style=\"color:#800000;\">expired</b>." )
		);
	}
	else if (timeLeft < intMax) {
		int daysLeft = timeLeft / (3600 * 24) + (timeLeft % (3600 * 24) >= (1800 * 24));
		label->setText(
			tr( "Pacu is currently running in <b>evaluation mode</b>.<br/>"
			    "There are still <b>%1 days left</b> before Pacu will stop operating.<br/>"
			    "(Press ESC to close this window.)" ).arg(daysLeft)
		);
	}
	else {
		label->setText(
			tr( "Pacu is <b style=\"color:#008000;\">fully activated</b>.<br/>"
			    "Thank you for your business." )
		);
	}
}

} // namespace pacu
