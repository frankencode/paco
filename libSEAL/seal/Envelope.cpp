#ifdef __APPLE__
#ifdef NDEBUG
#include <sys/types.h>
#include <sys/ptrace.h>
#endif
#endif
#include <ftl/streams>
#include <ftl/process>
#include <ftl/Crc32.hpp>
#include "Envelope.hpp"
#include "private.hpp"

namespace seal
{

int Envelope::count_ = 0;

Envelope::Envelope()
{
	if (count_ > 0) return;
	++count_;
	#ifdef __APPLE__
	#ifdef NDEBUG
	ptrace(PT_DENY_ATTACH, 0, 0, 0);
	#endif
	#endif
	if ((!verify(Process::execPath())) || (!checkCrc())) {
		#ifdef NDEBUG
			Process::sleep(60);
			Process::exit(0);
		#else
			Process::exit(73);
		#endif
	}
}

} // namespace seal
