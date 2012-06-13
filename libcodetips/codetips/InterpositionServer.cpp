/*
 * InterpositionServer.cpp -- file open interposition server
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include <unistd.h> // __MACH__?
#include <ftl/PrintDebug.hpp> // DEBUG
#include <ftl/Format.hpp>
#include <ftl/LinkInfo.hpp>
#include <ftl/LineSource.hpp>
#include <ftl/LineSink.hpp>
#include "InterpositionServer.hpp"

int codetips_hook;

namespace codetips
{

InterpositionServer::InterpositionServer()
{
	start();
}

InterpositionServer::~InterpositionServer()
{
	done_.release();
	try { kill(); } catch (...) {}
	wait();
}

String InterpositionServer::socketPath()
{
	return Format("/tmp/codetips_%%.socket") << Process::currentId();
}

void InterpositionServer::injectClient(Ref<EnvMap> map)
{
	map->insert("CODETIPS_SOCKET", socketPath());
	String libPath = LinkInfo((void*)&codetips_hook).libraryPath().replace("codetips.", "codetipsclient.");
	#ifdef __MACH__
	map->insert("DYLD_FORCE_FLAT_NAMESPACE", "");
	map->insert("DYLD_INSERT_LIBRARIES", libPath);
	#else
	map->insert("LD_PRELOAD", libPath);
	#endif
}

void InterpositionServer::run()
{
	try { File(socketPath()).unlink(); } catch(...) {}
	socket_ = new StreamSocket(new SocketAddress(AF_LOCAL, socketPath()));
	socket_->bind();
	socket_->listen();
	while (!done_.tryAcquire()) {
		try {
			Ref<StreamSocket, Owner> stream = socket_->accept();
			LineSource source(stream);
			LineSink sink(stream);
			String path = source.readLine();
			String redirPath = redirectOpen(path);
			sink.writeLine(redirPath);
			/* NB: May be signalled in two different areas:
			 *     [A] in kernel (accept(2), read(2) or write(2))
			 *     [B] during userlevel code execution (between the calls above)
			 * In case [A] direct termination is guaranteed (throwing/catching StreamException).
			 * In case [B] at most another connection will be accepted and handled completely.
			 */
		}
		catch (StreamException&) {
			break;
		}
	}
}

} // namespace codetips
