/*
 * HaxeMessageSyntax.hpp -- parser for haxe compiler messages
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_HAXEMESSAGESYNTAX_HPP
#define CODETIPS_HAXEMESSAGESYNTAX_HPP

#include <ftl/Syntax.hpp>
#include <ftl/Array.hpp>

namespace codetips
{

using namespace ftl;

class HaxeMessageSyntax: public Syntax<ByteArray>::Definition
{
public:
	HaxeMessageSyntax();
	
	Ref<Tip, Owner> parse(String message);
	
private:
	String readValue(String message, Ref<Token> token);
	static String docLink(String type);
	static String displayString(String type, Ref<Arguments> arguments = 0, bool broken = false);
	Ref<Type, Owner> readType(String typeString);
	
	int gt_, lt_;
	int specialChar_;
	int value_;
	int typeTip_;
	int memberName_;
	int memberType_;
	int memberDescription_;
	int member_;
	int membersTip_;
	int haxeTip_;
};

} // namespace codetips

#endif // CODETIPS_HAXEMESSAGESYNTAX_HPP
