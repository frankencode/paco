/*
 * XmlOpenTagSyntax.hpp -- syntax of an opening XML tag without closing '>'
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_XMLOPENTAGSYNTAX_HPP
#define CODETIPS_XMLOPENTAGSYNTAX_HPP

#include <ftl/Syntax.hpp>
#include <ftl/Array.hpp>

namespace codetips
{

using namespace ftl;

class XmlOpenTagSyntax: public Syntax<ByteArray>::Definition
{
public:
	XmlOpenTagSyntax();
};

} // namespace codetips

#endif // CODETIPS_XMLOPENTAGSYNTAX_HPP
