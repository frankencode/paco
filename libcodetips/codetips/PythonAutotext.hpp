/*
 * PythonAutotext.hpp -- Python autotext generator
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_PYTHONAUTOTEXT_HPP
#define CODETIPS_PYTHONAUTOTEXT_HPP

#include "codetips.hpp"

namespace codetips
{

class PythonAutotext: public Assistant
{
public:
	PythonAutotext();
	
	String language() const;
	String name() const;
	String displayName() const;
	String description() const;
	Ref<Tip, Owner> assist(Ref<Context> context, int modifiers, uchar_t key);
};

} // namespace codetips

#endif // CODETIPS_PYTHONAUTOTEXT_HPP
