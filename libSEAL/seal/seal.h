#ifndef SEAL_SEAL_H
#define SEAL_SEAL_H

#ifdef __cplusplus
extern "C" {
#endif

uint32_t sealTrialKey();
int sealTrialActivate(uint32_t trialKey, int trialPeriod, const char* settingsPath, bool subscription = false);

enum SealVerificationError {
	NoError = 0,
	RedundantActivation = 9,
	
	OutOfMemory = 1,
	TransmissionError = 2,
	FailedForUnknownReason = 3,
	UnknownLicenseKey = 4,
	InvalidLicenseKey = 5,
	InternalError = 6,
	AnnualActivationLimit = 7,
	NetworkInterfaceLimit = 8,
	InvalidRequest = 9
};

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
);

void sealFullActivationCode(void** mid, int* midLen);
int sealFullActivate(void* mid, int midLen);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SEAL_SEAL_H
