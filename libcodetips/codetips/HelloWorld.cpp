/*
 * HelloWorld.cpp -- dummy assistant for testing
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include <ftl/PrintDebug.hpp>
#include "AssistantManager.hpp"
#include "HelloWorld.hpp"

namespace codetips
{

// CODETIPS_REGISTRATION(HelloWorld)

HelloWorld::HelloWorld()
{
	printTo(log(), "All fine.\n");
}

String HelloWorld::language() const { return "plain"; }
String HelloWorld::name() const { return "test"; }
String HelloWorld::displayName() const { return "Test Assistant"; }
String HelloWorld::description() const { return "Testing module. Says 'Hello' when pressing the magic key (Alt-Tab)."; }

Ref<Tip, Owner> HelloWorld::assist(Ref<Context> context, int modifiers, uchar_t key)
{
	Ref<Tip, Owner> tip;
	if ((modifiers == Alt) && (key == '\t'))
		tip = new TypeTip(new Type("Hello!"));
	return tip;
}

} // namespace codetips
