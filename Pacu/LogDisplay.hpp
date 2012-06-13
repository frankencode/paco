#ifndef PACU_LOGDISPLAY_HPP
#define PACU_LOGDISPLAY_HPP

#include <ftl/String.hpp>
#include <ftl/Thread.hpp>
#include "UseFtl.hpp"

namespace ftl {
	class Semaphore;
}

namespace pacu
{

class LogDisplay: public Thread
{
public:
	LogDisplay(String path);
	~LogDisplay();
	
	void finish();
	
private:
	virtual void run();
	
	String path_;
	Ref<Semaphore, Owner> finish_;
};

} // namespace pacu

#endif // PACU_LOGDISPLAY_HPP
