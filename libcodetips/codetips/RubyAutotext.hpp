/*
 * RubyAutotext.hpp -- Ruby autotext generator
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_RUBYAUTOTEXT_HPP
#define CODETIPS_RUBYAUTOTEXT_HPP

#include "codetips.hpp"

namespace codetips
{

class RubyAutotext: public Assistant {
public:
	RubyAutotext();
	
	String language() const;
	String name() const;
	String displayName() const;
	String description() const;
	Ref<Tip, Owner> assist(Ref<Context> context, int modifiers, uchar_t key);
};

} // namespace codetips

#endif // CODETIPS_RUBYAUTOTEXT_HPP
