/*
 * HelloWorld.hpp -- dummy assistant for testing
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_HELLOWORLD_HPP
#define CODETIPS_HELLOWORLD_HPP

#include "codetips.hpp"

namespace codetips
{

class HelloWorld: public Assistant {
public:
	HelloWorld();
	
	virtual String language() const;
	virtual String name() const;
	virtual String displayName() const;
	virtual String description() const;
	virtual Ref<Tip, Owner> assist(Ref<Context> context, int modifiers, uchar_t key);
};

} // namespace codetips

#endif // CODETIPS_HELLOWORLD_HPP
