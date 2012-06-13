/*
 * HaxeCodetips.hpp -- haxe codetips generator
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_HAXEINSTRUCTOR_HPP
#define CODETIPS_HAXEINSTRUCTOR_HPP

#include "codetips.hpp"

namespace ftl {
	class ProcessFactory;
}

namespace codetips
{

class HaxeMessageSyntax;

class HaxeCodetips: public Assistant {
public:
	HaxeCodetips();
	
	String language() const;
	String name() const;
	String displayName() const;
	String description() const;
	
	Ref<Tip, Owner> assist(Ref<Context> context, int modifiers, uchar_t key);
	
private:
	virtual void update();
	
	Ref<ProcessFactory, Owner> processFactory_;
	Ref<HaxeMessageSyntax, Owner> messageSyntax_;
};

} // namespace codetips

#endif // CODETIPS_HAXEINSTRUCTOR_HPP
