#ifndef SEAL_ACTIVATION_HPP
#define SEAL_ACTIVATION_HPP

#include <inttypes.h>

namespace seal
{

class Activation
{
public:
	static bool touchFirstTime(int buildNumber = 0, bool* readonlyMedia = 0);
	static uint32_t trialKey();
	static int trialActivate(uint32_t trialKey, int trialPeriod, const char* settingsPath, bool readonlyMedia = false, bool upgrade = false, bool subscription = false);
	
	enum VerificationError {
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
	
	static bool verifyLicenseKey(
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
	
	static void fullActivationCode(void** mid, int* midLen, bool normalizedName = true);
	static int fullActivate(void* mid, int midLen);
	
	static char* curlVersion();
};

} // namespace seal

#endif // SEAL_ACTIVATION_HPP
