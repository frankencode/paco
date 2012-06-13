/*
 * InterpositionServer.hpp -- file open interposition server
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_INTERPOSITIONSERVER_HPP
#define CODETIPS_INTERPOSITIONSERVER_HPP

#include <ftl/String.hpp>
#include <ftl/Thread.hpp>
#include <ftl/Process.hpp>
#include <ftl/StreamSocket.hpp>
#include <ftl/Semaphore.hpp>

namespace codetips
{

using namespace ftl;

class InterpositionServer: public Thread
{
public:
	InterpositionServer();
	~InterpositionServer();
	
	static String socketPath();
	static void injectClient(Ref<EnvMap> map);
	virtual String redirectOpen(String path) = 0;
	
private:
	virtual void run();
	
	Ref<StreamSocket, Owner> socket_;
	Semaphore done_;
};

} // namespace codetips

#endif // CODETIPS_INTERPOSITIONSERVER_HPP
