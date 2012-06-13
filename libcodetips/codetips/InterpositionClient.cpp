/*
 * InterpositionClient.cpp -- file open interposition client
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include <unistd.h> // __MACH__?
#include <dlfcn.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// #include <ftl/PrintDebug.hpp> // DEBUG
#include <ftl/Path.hpp>
#include <ftl/LocalStatic.hpp>
#include <ftl/Process.hpp>
#include <ftl/StreamSocket.hpp>
#include <ftl/LineSink.hpp>
#include <ftl/LineSource.hpp>
#include "InterpositionClient.hpp"

namespace codetips
{

String InterpositionClient::redirectOpen(String path, int flags)
{
	Ref<SocketAddress, Owner> address = new SocketAddress(AF_LOCAL, Process::env("CODETIPS_SOCKET"));
	Ref<StreamSocket, Owner> stream = new StreamSocket(address);
	stream->connect();
	LineSink sink(stream);
	LineSource source(stream);
	sink.writeLine(Path(path).absolute());
	return source.readLine();
}

class OpenFunction {
public:
	typedef int (*Pointer)(const char* path, int flags, ...);
	OpenFunction()
		: ptr_((Pointer)dlsym(RTLD_NEXT, "open"))
	{}
	Pointer ptr_;
};

} // namespace codetips

// __attribute__((visibility("default")))
int open(const char* path, int flags, ...)
{
	using namespace codetips;
	String redirPath = InterpositionClient::redirectOpen(path, flags);
	OpenFunction& openFunction = localStatic<OpenFunction, InterpositionClient>();
	int ret = -1;
	if (flags & O_CREAT) {
		va_list ap;
		va_start(ap, flags);
		mode_t mode = va_arg(ap, int);
		va_end(ap);
		ret = openFunction.ptr_(redirPath, flags, mode);
	}
	else {
		ret = openFunction.ptr_(redirPath, flags);
	}
	return ret;
}
