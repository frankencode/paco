#include "Activation.hpp"

using namespace seal;

uint32_t sealTrialKey() { return Activation::trialKey(); }
int sealTrialActivate(uint32_t trialKey, int trialPeriod, const char* settingsPath, bool subscription) {
	return Activation::trialActivate(trialKey, trialPeriod, settingsPath, subscription);
}

bool sealVerifyLicenseKey(
	const char* url,
	const char* user,
	const char* pwd,
	const char* licenseHolder,
	const char* licenseKey,
	void* mid,
	int midLen,
	char** realLicenseHolder,
	int* error
)
{
	return Activation::verifyLicenseKey(url, user, pwd, licenseHolder, licenseKey, mid, midLen, realLicenseHolder, error);
}

void sealFullActivationCode(void** mid, int* midLen) {
	Activation::fullActivationCode(mid, midLen);
}

int sealFullActivate(void* mid, int midLen) {
	return Activation::fullActivate(mid, midLen);
}
