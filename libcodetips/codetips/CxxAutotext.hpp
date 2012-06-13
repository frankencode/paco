/*
 * CxxAutotext.hpp -- C++ autotext generator
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_CXXAUTOTEXT_HPP
#define CODETIPS_CXXAUTOTEXT_HPP

#include "codetips.hpp"

namespace codetips
{

class CxxAutotext: public Assistant {
public:
	CxxAutotext();
	
	String language() const;
	String name() const;
	String displayName() const;
	String description() const;
	Ref<Tip, Owner> assist(Ref<Context> context, int modifiers, uchar_t key);
};

} // namespace codetips

#endif // CODETIPS_CXXAUTOTEXT_HPP
