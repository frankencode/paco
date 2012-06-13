/*
 * InterpositionClient.hpp -- file open interposition client
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_INTERPOSITIONCLIENT_HPP
#define CODETIPS_INTERPOSITIONCLIENT_HPP

#include <ftl/String.hpp>

namespace codetips
{

using namespace ftl;

class InterpositionClient
{
public:
	static String redirectOpen(String path, int flags);
	static int open(const char* path, int flags);
};

} // namespace codetips

extern "C" {
	int open(const char* path, int flags, ...);
}

#endif // CODETIPS_INTERPOSITIONCLIENT_HPP
