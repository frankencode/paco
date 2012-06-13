#ifndef PACU_QXACTIVATION_HPP
#define PACU_QXACTIVATION_HPP

#include <QByteArray>
#include <QSettings>
#include <QThread>
#include <seal/Activation.hpp>
#include "UseFtl.hpp"

class QProgressDialog;

namespace pacu
{

class QxActivation: public QThread, public seal::Activation, public Instance
{
	Q_OBJECT
	
public:
	static QxActivation* instance();
	
	void restoreState(QSettings* settings);
	void saveState(QSettings* settings);
	
	int timeLeft() const;
	
	QString licenseHolder() const;
	QString licenseKey() const;
	
	bool verifyLicenseKey(int* error, char** realLicenseHolder, QProgressDialog* progress);
	
public slots:
	void setLicenseHolder(const QString& name);
	void setLicenseKey(const QString& key);
	
private:
	virtual void run();
	static Ref<QxActivation, Owner> instance_;
	int timeLeft_;
	QString licenseHolder_;
	QString licenseKey_;
	QByteArray mid_;
	int* error_;
	char** realLicenseHolder_;
	bool ok_;
	QxActivation();
	uint32_t crc32(const QSettings* settings) const;
};

inline QxActivation* activation() { return QxActivation::instance(); }

} // namespace pacu

#endif // PACU_QXACTIVATION_HPP
