#include <ftl/atoms>
#include <ftl/process>
#include <ftl/streams>
#include <ftl/Crc32.hpp>
#include <ftl/Random.hpp>
#include <ftl/NetworkInterface.hpp>
#include <ftl/Process.hpp>
#include <ftl/User.hpp>
#ifndef __linux
#include <sys/sysctl.h> // sysctlnametomib, sysctl
#include <string.h> // memset
#endif
#include <stdio.h>
#include <stdlib.h> // malloc, free
#include <curl/curl.h>
#include "Envelope.hpp"
#include "Activation.hpp"
#include "endian.hpp"

namespace seal
{

#define DEBUG_LOG debug // print

using namespace ftl;

/** \brief check if this executable is called the first time
  *
  * Checks if this executable is called the first time.
  * The persistent media, on which the executable is stored on, is changed in a way
  * that any subsequent invocation of this method will return false.
  * If the media is readonly this function always returns true.
  */
bool Activation::touchFirstTime(int buildNumber, bool* readonlyMedia)
{
	FileStatus status(Process::execPath());
	Time tm = status.lastModified().s();
	Random rand(buildNumber);
	int x = rand.get() & 63;
	bool firstTime = ((tm.sec() & 63) != x);
	if (firstTime) {
		try {
			tm = Time((tm.sec() & (~63)) | x, tm.nsec());
			status.setTimes(status.lastAccess(), tm);
			if (readonlyMedia)
				*readonlyMedia = false;
		}
		catch (Exception& ex) {
			if (readonlyMedia)
				*readonlyMedia = true;
		}
	}
	return firstTime;
}

/** \brief trial code generator
  *
  * Generates a trial code to be used for trial activation.
  * The trial code should be generated only once when the application
  * is launched the first time. It is assumed, that the trial code
  * is stored in the settings file.
  */
uint32_t Activation::trialKey()
{
	Crc32 crc;
	String loginName = ProcessStatus(Process::currentId()).loginName();
	crc.feed(loginName->data(), loginName->size());
	FileStatus status(Process::execPath());
	uint32_t tm = status.lastModified().s();
	ino_t in = status.inodeNumber();
	tm = sealEndianGate(tm);
	in = sealEndianGate(in);
	crc.feed(&tm, sizeof(tm));
	crc.feed(&in, sizeof(in));
	return crc.sum();
}

/** \brief trial activate
  *
  * Activate this session for a given trial period.
  *
  * \arg trialKey license key for this trial period
  * \arg trialPeriod maximum duration of the trial period (in seconds)
  * \arg settingsPath path to a settings file (plist, conf, etc...), should store the trial code
  * \arg readonlyMedia executable launched from a readonly media
  * \arg upgrade executable was upgraded
  * \arg subscription evaluation period starts from build date of the executable
  * \ret time left in seconds if activation was successful, else -1
  */
int Activation::trialActivate(uint32_t trialKey, int trialPeriod, const char* settingsPath, bool readonlyMedia, bool upgrade, bool subscription)
{
	Ref<FileStatus, Owner> exe = new FileStatus(Process::execPath());
	Time tem = exe->lastModified();
	Time tea = exe->lastAccess();
	Time tec = exe->lastChanged();
	Time t1 = Time::now();
	
	DEBUG_LOG("(seal) tem, tea, tec = %%(%%), %%(%%), %%(%%)\n",
		tem.s(), tem.s() - t1.s(),
		tea.s(), tea.s() - t1.s(),
		tec.s(), tec.s() - t1.s()
	);
	
	if (!readonlyMedia)
	{
		if (((tem - t1).min() > 5) || ((tea - t1).min() > 5) || ((tec - t1).min() > 5)) {
			DEBUG_LOG("(seal) Trial activation rejected: Program has been installed in the future\n");
			return -1;
		}
		
		if (trialKey != Activation::trialKey()) {
			DEBUG_LOG("(seal) Trial activation rejected: Executable has been copied to a different location (most likely)\n");
			return -1;
		}
	}
	
	int timeLeft = trialPeriod;
	if (subscription || (!readonlyMedia))
		timeLeft -= (subscription ? t1 - tem : t1 - tec).s();
	
	try {
		Ref<FileStatus, Owner> stt = new FileStatus(settingsPath);
		Time tsm = stt->lastModified();
		Time tsa = stt->lastAccess();
		Time tsc = stt->lastChanged();
		
		DEBUG_LOG("(seal) tsm, tsa, tsc = %%, %%, %%\n", tsm.s() - t1.s(), tsa.s() - t1.s(), tsc.s() - t1.s());
		
		if (tsm > t1) {
			DEBUG_LOG("(seal) Trial activation rejected: Last session ended in the future (system clock turned back?)\n");
			return -1;
		}
		
		/*if (tsm < tem) {
			DEBUG_LOG("(seal) Trial activation rejected: Settings file is older than executable\n");
			return -1;
		}*/
		
		if ((!subscription) && (!upgrade) && (!readonlyMedia))
			timeLeft = trialPeriod - ((tec < tsc) ? (t1 - tec) : (t1 - tsc)).s();
		
		if ((!subscription) && readonlyMedia)
			timeLeft = trialPeriod - (t1 - tsc).s();
	}
	catch (SystemException& ex)
	{
		// in case settings file does not exists, yet
	}
	
	if (timeLeft > trialPeriod) {
		DEBUG_LOG("(seal) Trial activation warning: time left exceeds trial period");
		timeLeft = trialPeriod;
	}
	
	DEBUG_LOG("(seal) Trial activation granted: %% s of trial period left (upgrade = %%)\n", timeLeft, upgrade);
	
	return timeLeft;
}

/** \intern
  */
String cpuModelName()
{
	String modelName;
	#ifdef __linux
	Ref<File, Owner> file = new File("/proc/cpuinfo");
	file->open(File::Read);
	Ref<LineSource, Owner> source = new LineSource(file);
	for (String ln; source->read(&ln);) {
		Ref<StringList, Owner> parts = ln.split(":");
		if (parts->length() == 2) {
			String key = parts->at(0).stripTrailingSpace();
			String value = parts->at(1).stripLeadingSpace();
			if (key == "model name") {
				modelName = value;
				break;
			}
		}
	}
	#else
	int mib[4]; size_t len = 4, sz = 0;
	if (::sysctlnametomib("machdep.cpu.brand_string", mib, &len) == -1)
		FTL_SYSTEM_EXCEPTION;
	if (::sysctl(mib, len, NULL, &sz, NULL, NULL) == -1)
		FTL_SYSTEM_EXCEPTION;
	char* cs = (char*)::malloc(sz);
	::memset(cs, 0, sz);
	if (::sysctl(mib, len, cs, &sz, NULL, NULL) == -1)
		FTL_SYSTEM_EXCEPTION;
	modelName = cs;
	::free(cs);
	#endif
	return modelName;
}

/** \brief verify license key
  *
  * This method is not thread-safe!
  */
bool Activation::verifyLicenseKey(
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
	DEBUG_LOG("(seal) Sending verification request to the sales server...\n");
	
	CURL* curl = curl_easy_init();
	if (!curl) {
		if (error) *error = OutOfMemory;
		return false;
	}
	
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "SEAL0");
	/*curl_easy_setopt(curl, CURLOPT_USERNAME, user);
	curl_easy_setopt(curl, CURLOPT_PASSWORD, pwd);*/
	char login[256];
	snprintf(login, 256, "%s:%s", user, pwd);
	curl_easy_setopt(curl, CURLOPT_USERPWD, login);
	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	#ifndef NDEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	#endif
	
	String midString;
	{
		Format h;
		uint32_t* m = (uint32_t*)mid;
		for (int i = 0, n = midLen / sizeof(uint32_t); i < n; ++i) {
			String s = Format("0x%hex%") << sealEndianGate(m[i]);
			h << s;
			if (i < n - 1)
				h << ",";
		}
		midString = h;
	}
	DEBUG_LOG("(seak) midString = \"%%\"\n", midString);
	
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	curl_formadd(
		&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "license_holder",
		CURLFORM_COPYCONTENTS, licenseHolder,
		CURLFORM_END
	);
	curl_formadd(
		&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "license_key",
		CURLFORM_COPYCONTENTS, licenseKey,
		CURLFORM_END
	);
	curl_formadd(
		&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "machine_ids",
		CURLFORM_COPYCONTENTS, (char*)midString,
		CURLFORM_END
	);
	curl_formadd(
		&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "commit",
		CURLFORM_COPYCONTENTS, "register",
		CURLFORM_END
	);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
	
	Ref<File, Owner> file = new File("/tmp/seal1XXXXXX");
	file->createUnique();
	file->unlinkOnExit();
	file->open(File::Read|File::Write);
	FILE* stream = ::fdopen(::dup(file->fd()), "r+");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
	#ifndef __linux
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	#endif
	
	CURLcode res = curl_easy_perform(curl);
	
	DEBUG_LOG("(seal) res = %%\n", res);
	if (res != 0) {
		*error = TransmissionError;
		return false;
	}
	
	char* url2 = 0;
	res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url2); // CURLINFO_REDIRECT_URL
	if (res != 0) {
		*error = TransmissionError;
		return false;
	}
	DEBUG_LOG("(seal) url2 = \"%%\"\n", url2);
	
	bool success = String(url2).contains("success");
	
	::fclose(stream);
	
	file = new File(file->path());
	file->open(File::Read);
	Ref<LineSource, Owner> source = new LineSource(file);
	String line1, line2;
	for (String line; source->read(&line);) {
		DEBUG_LOG("(seal) line = \"%%\"\n", line);
		Ref<StringList, Owner> parts = line.split("=");
		if (parts->length() == 2) {
			String key = parts->at(0);
			String value = parts->at(1);
			if (key == "error_code") {
				*error = value.toInt();
				if ((*error != NoError) && (*error != RedundantActivation)) success = false;
				DEBUG_LOG("(seal) error = %%\n", *error);
			}
			else if (key == "license_holder") {
				*realLicenseHolder = str::dup(value->data());
				DEBUG_LOG("(seal) license_holder = %%\n", *realLicenseHolder);
			}
		}
	}
	
	if (success && (*error != RedundantActivation)) *error = NoError;
	curl_easy_cleanup(curl);
	curl_formfree(formpost);
	
	return success;
}

/** \brief Generate machine id
  *
  * Generates a machine id to be used for full product activation.
  * The machine id should be generated only once when the application
  * is activated by a registered customer. It is assumed, that the machine id
  * is stored in the settings file.
  */
void Activation::fullActivationCode(void** mid, int* midLen, bool normalizedName)
{
	typedef List<uint64_t> AddressList;
	Ref<AddressList, Owner> addressList = new AddressList;
	Ref<NetworkInterfaceList, Owner> interfaceList = NetworkInterface::queryAll(-1);
	for (int i = 0; i < interfaceList->length(); ++i) {
		uint64_t addr = interfaceList->get(i)->hardwareAddress();
		if ((addr != 0) && (!addressList->contains(addr)))
			addressList->append(addr);
	}
	
	const int bufSize = sizeof(uint32_t) * 6;
	uint32_t* buf = (uint32_t*)::malloc(bufSize);
	int bufFill = 0;
	String userName = User(Process::realUserId()).fullName();
	if (normalizedName) userName = userName.normalized();
	String cpuModel = seal::cpuModelName();
	DEBUG_LOG("(seal) userName = %%\n", userName);
	DEBUG_LOG("(seal) cpuModel = \"%%\"\n", cpuModel);
	buf[bufFill++] = sealEndianGate(crc32(userName->data(), userName->size()));
	buf[bufFill++] = sealEndianGate(crc32(cpuModel->data(), cpuModel->size()));
	for (int i = 0; (i < addressList->length()) && (i < 4); ++i) {
		Crc32 crc;
		uint64_t addr = addressList->at(i);
		DEBUG_LOG("(seal) addressList->at(%%) = %hex%\n", i, addr);
		addr = sealEndianGate(addr);
		crc.feed(&addr, sizeof(addr));
		buf[bufFill++] = sealEndianGate(crc.sum());
	}
	while (bufFill < 6)
		buf[bufFill++] = 0;
	
	DEBUG_LOG("(seal) (");
	for (int i = 0; i < bufFill; ++i)
		DEBUG_LOG("%hex% ", sealEndianGate(buf[i]));
	DEBUG_LOG(")\n");
	
	*mid = (void*)buf;
	*midLen = bufSize;
}

/** \brief full product activation
  *
  * Activate this session for unlimited usage.
  *
  * \arg mid machine id
  * \arg midLen as returned by fullActivationCode()
  * \ret >0 if successful, else -1
  */
int Activation::fullActivate(void* mid, int midLen)
{
	if (midLen != 6 * sizeof(uint32_t)) {
		DEBUG_LOG("(seal) Full activation rejected: Illegal midLen\n");
		return -1;
	}
	uint32_t* buf = (uint32_t*)mid;
	String userName = User(Process::realUserId()).fullName();
	String cpuModel = seal::cpuModelName();
	int bufFill = 0;
	if (sealEndianGate(buf[bufFill++]) != crc32(userName->data(), userName->size())) {
		DEBUG_LOG("(seal) Full activation rejected: User name does not match\n");
		return -1;
	}
	if (sealEndianGate(buf[bufFill++]) != crc32(cpuModel->data(), cpuModel->size())) {
		DEBUG_LOG("(seal) Full activation rejected: CPU model does not match\n");
		return -1;
	}
	
	Ref<NetworkInterfaceList, Owner> interfaceList = NetworkInterface::queryAll(-1);
	bool oneMatch = false;
	while ((bufFill < 6) && (!oneMatch)) {
		uint32_t sum = sealEndianGate(buf[bufFill++]);
		for (int i = 0; i < interfaceList->length(); ++i) {
			Ref<NetworkInterface> interface = interfaceList->at(i);
			uint64_t addr = sealEndianGate(interface->hardwareAddress());
			if (addr != 0) {
				Crc32 crc;
				crc.feed(&addr,sizeof(addr));
				if (sum == crc.sum()) {
					oneMatch = true;
					break;
				}
			}
		}
	}
	
	if (!oneMatch) {
		DEBUG_LOG("(seal) Full activation rejected: Machine identity does not match\n");
		return -1;
	}
	
	return intMax;
}

char* Activation::curlVersion() { return curl_version(); }

} // namespace seal
