#include <ftl/File.hpp>
#include <ftl/FileStatus.hpp>
#include <ftl/StandardStreams.hpp>
#include <ftl/Semaphore.hpp>
#include "LogDisplay.hpp"

namespace pacu
{

LogDisplay::LogDisplay(String path)
	: path_(path),
	  finish_(new Semaphore)
{
	start();
}

LogDisplay::~LogDisplay()
{
	finish_->release();
	wait();
}

void LogDisplay::run()
{
	rawOutput()->write("\033[2J\033[H");
	Ref<File, Owner> file = new File(path_, File::Read);
	off_t nw = 0;
	Ref<ByteArray, Owner> buf = new ByteArray(FTL_DEFAULT_BUF_CAPA);
	while (true) {
		if (file->status()->size() > nw) {
			while (true) {
				int nr = file->readAvail(buf);
				if (nr == 0) break;
				nw += nr;
				rawOutput()->write(buf->data(), nr);
			}
		}
		if (finish_->acquireBefore(Time::now() + 1))
			break;
		file->status()->update();
	}
}

} // namespace pacu
