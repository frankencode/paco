#include <QStringList>
#include <QApplication>
#include <QProgressDialog>
#include <ftl/Crc32.hpp>
#include <ftl/FileStatus.hpp>
#include <ftl/Process.hpp>
#include <ftl/User.hpp>
#include <ftl/Random.hpp>
#include <ftl/String.hpp>
#include <ftl/streams> // DEBUG
#include "UseFtl.hpp"
#include "QxActivation.hpp"

namespace pacu
{

#define DEBUG_LOG debug // print

/** Sequential build number.
  * Shareware users upgrading to a new build will
  * get a new trial key generated (trial period starts anew).
  * 0..2: Pacu 1.0
  * 3..19: Pacu 1.1
  */
enum { BuildNumber =
	#include "BuildNumber"
};

Ref<QxActivation, Owner> QxActivation::instance_ = 0;

QxActivation::QxActivation()
	: timeLeft_(-1)
{}

QxActivation* QxActivation::instance()
{
	if (!instance_)
		instance_ = new QxActivation;
	return instance_;
}

void QxActivation::restoreState(QSettings* settings)
{
	if (settings->contains("licenseHolder"))
		licenseHolder_ = settings->value("licenseHolder").toString();
	else
		licenseHolder_ = User(Process::realUserId()).fullName();
	
	if (settings->contains("licenseKey"))
		licenseKey_ = settings->value("licenseKey").toString();
	
	bool validCrc = false;
	if (settings->contains("crc")) {
		uint32_t sum = settings->value("crc").toUInt();
		uint32_t checkSum = crc32(settings);
		validCrc = (sum == checkSum);
		DEBUG_LOG("QxActivation::restoreState(): sum, checkSum = %hex%, %hex%\n", sum, checkSum);
	}
	if (!validCrc)
		settings->clear();
	
	bool trialMode = true;
	
	if (settings->contains("mid")) {
		QByteArray mid = settings->value("mid").toByteArray();
		trialMode = !fullActivate(mid.data(), mid.size());
		if (!trialMode) timeLeft_ = intMax;
	}
	
	if (trialMode) {
		uint32_t key = 0;
		bool readonlyMedia = false;
		bool firstTime = touchFirstTime(BuildNumber, &readonlyMedia);
		bool upgrade = settings->contains("buildNumber") ? (settings->value("buildNumber").toInt() != BuildNumber) : firstTime;
		bool generateKey = (firstTime && (!settings->contains("trialKey"))) || upgrade;
		DEBUG_LOG("QxActivation::restoreState(): firstTime, upgrade = %%, %%\n", firstTime, upgrade);
		if (generateKey) {
			DEBUG_LOG("QxActivation::restoreState(): Generating new activation key\n");
			key = Activation::trialKey();
		}
		else
			key = settings->value("trialKey").toUInt();
		timeLeft_ = trialActivate(key, 30 * 24 * 3600 + 11, settings->fileName().toUtf8().constData(), readonlyMedia, upgrade);
		if (!readonlyMedia) {
			settings->setValue("trialKey", key);
			settings->setValue("buildNumber", BuildNumber);
		}
	}
}

void QxActivation::saveState(QSettings* settings)
{
	settings->setValue("licenseHolder", licenseHolder_);
	settings->setValue("licenseKey", licenseKey_);
	if (!mid_.isNull())
		settings->setValue("mid", mid_);
	uint32_t sum = crc32(settings);
	settings->setValue("crc", sum);
	DEBUG_LOG("QxActivation::saveState(): sum = %hex%\n", sum);
}

int QxActivation::timeLeft() const { return timeLeft_; }

QString QxActivation::licenseHolder() const { return licenseHolder_; }
QString QxActivation::licenseKey() const { return licenseKey_; }

bool QxActivation::verifyLicenseKey(int* error, char** realLicenseHolder, QProgressDialog* progress)
{
	error_ = error;
	realLicenseHolder_ = realLicenseHolder;
	ok_ = false;
	start();
	while (!wait(100)) {
		if (progress->value() == progress->maximum()) break;
		if (progress->wasCanceled()) break;
		progress->setValue(progress->value() + 1);
		QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	}
	if (progress->value() != progress->maximum())
		progress->setValue(progress->maximum());
	QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	return ok_;
}

void QxActivation::run()
{
	bool normalizedName = true;
	void* mid = 0;
	int midLen = 0;
	while (true) {
		fullActivationCode(&mid, &midLen, normalizedName);
		ok_ = Activation::verifyLicenseKey(
			"http://www.cyblogic.com/en/activations/register",
			"pacu1", "salespoint7",
			licenseHolder_.toUtf8().data(),
			licenseKey_.toLower().simplified().toUtf8().data(),
			mid, midLen,
			realLicenseHolder_,
			error_
		);
		if (ok_) {
			mid_ = QByteArray((char*)mid, midLen);
			timeLeft_ = intMax;
			::free(mid);
			break;
		}
		else {
			::free(mid);
			mid = 0;
			midLen = 0;
			if (!normalizedName) break;
			// compatibility HACK, try a second time to support
			// activation from older versions of Pacu (< 1.2)
			normalizedName = false;
		}
	}
}

void QxActivation::setLicenseHolder(const QString& name) { licenseHolder_ = name; }
void QxActivation::setLicenseKey(const QString& key) { licenseKey_ = key; }

uint32_t QxActivation::crc32(const QSettings* settings) const
{
	Crc32 crc;
	QStringList keys = settings->allKeys();
	for (int i = 0, n = keys.count(); i < n; ++i) {
		QString key = keys.at(i);
		if (key != "crc") {
			QString value = settings->value(key).toString();
			if (value.length() != 0) {
				QByteArray ba = value.toUtf8();
				crc.feed(ba.data(), ba.size());
			}
		}
	}
	return crc.sum();
}

} // namespace pacu
