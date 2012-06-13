/*
 * HaxeMessageSyntax.cpp -- parser for haxe compiler messages
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include <ftl/PrintDebug.hpp> // DEBUG
#include "codetips.hpp"
#include "HaxeMessageSyntax.hpp"

namespace codetips
{

HaxeMessageSyntax::HaxeMessageSyntax()
{
	DEFINE_VOID("SpaceChar", RANGE(" \n\r\t"));
	DEFINE_VOID("Whitespace", REPEAT(INLINE("SpaceChar")));
	
	specialChar_ =
		DEFINE("SpecialChar",
			GLUE(
				CHAR('&'),
				KEYWORD("gt lt"),
				CHAR(';')
			)
		);
	gt_ = keywordByName("gt");
	lt_ = keywordByName("lt");
	
	value_ =
		DEFINE("Value",
			REPEAT(1,
				CHOICE(
					REF("SpecialChar"),
					GLUE(
						NOT(
							GLUE(
								INLINE("Whitespace"),
								CHAR('<')
							)
						),
						ANY()
					)
				)
			)
		);
	
	typeTip_ =
		DEFINE("TypeTip",
			GLUE(
				STRING("<type>"),
				INLINE("Whitespace"),
				REF("Value"),
				INLINE("Whitespace"),
				STRING("</type>")
			)
		);
	
	memberName_ = DEFINE("MemberName", REPEAT(1, OTHER('\"')));
	memberType_ = DEFINE("MemberType", REPEAT(0, 1, INLINE("Value")));
	memberDescription_ = DEFINE("MemberDescription", REPEAT(0, 1, INLINE("Value")));
	
	member_ =
		DEFINE("Member",
			GLUE(
				STRING("<i n=\""),
				REF("MemberName"),
				STRING("\"><t>"),
				REF("MemberType"),
				STRING("</t><d>"),
				REF("MemberDescription"),
				STRING("</d></i>")
			)
		);
	
	membersTip_ =
		DEFINE("MembersTip",
			GLUE(
				STRING("<list>"),
				REPEAT(1,
					GLUE(
						INLINE("Whitespace"),
						REF("Member")
					)
				),
				INLINE("Whitespace"),
				STRING("</list>")
			)
		);
	
	haxeTip_ =
		DEFINE("HaxeTip",
			FIND(
				GLUE(
					INLINE("Whitespace"),
					CHOICE(
						REF("TypeTip"),
						REF("MembersTip")
					),
					INLINE("Whitespace")
				)
			)
		);
	
	ENTRY("HaxeTip");
}

String HaxeMessageSyntax::readValue(String message, Ref<Token> token)
{
	if (token->firstChild()) {
		StringList list;
		Ref<Token> child = token->firstChild();
		int i = token->i0();
		while (child) {
			if (i < child->i0())
				list.append(message->copy(i, child->i0()));
			String s;
			if (child->keyword() == gt_) s = ">";
			else if (child->keyword() == lt_) s = "<";
			else s = message->copy(child->i0(), child->i1());
			list.append(s);
			i = child->i1();
			child = child->nextSibling();
		}
		if (i < token->i1())
			list.append(message->copy(i, token->i1()));
		return list.join();
	}
	else
		return message->copy(token->i0(), token->i1());
}

String HaxeMessageSyntax::docLink(String type)
{
	Ref<StringList, Owner> parts = type.split(".");
	if (parts->length() > 1) {
		String path;
		if (parts->at(0) == "haxe")
			path = Format("http://haxe.org/api/%%") << parts->join("/");
		else if (parts->at(0) == "flash")
			path = Format("http://help.adobe.com/en_US/AS3LCR/Flash_10.0/%%.%%") << parts->join("/") << "html";
		if (path != "")
			type = Format("<a href=\"%%\">%%</a>") << path << type;
	}
	return type;
}

String HaxeMessageSyntax::displayString(String type, Ref<Arguments> arguments, bool broken)
{
	Ref<StringList, Owner> parts = new StringList;
	if (arguments) {
		*parts << "function(";
		if (broken)
			*parts << "<br/>";
		for (int i = 0; i < arguments->length(); ++i) {
			Ref<Argument> argument = arguments->at(i);
			if (broken)
				*parts << "&nbsp;&nbsp;&nbsp;&nbsp;";
			if (argument->name() != "")
				*parts << argument->name() << ": ";
			*parts << docLink(argument->type());
			if (arguments->has(i + 1))
				*parts << ", ";
			if (broken)
				*parts << "<br/>";
		}
		*parts << "): ";
	}
	*parts << docLink(type);
	if (broken)
		*parts << "<br/>";
	String s = parts->join();
	if ((s->size() > 80) && (!broken)) {
		if (s.stripTags()->size() > 80)
			s = displayString(type, arguments, true);
	}
	return s;
}

Ref<Type, Owner> HaxeMessageSyntax::readType(String typeString)
{
	Ref<Type, Owner> type;
	Ref<StringList, Owner> parts = typeString.split(" ")->join().split("->");
	if (parts->length() > 1) {
		Ref<Arguments, Owner> arguments = new Arguments(parts->length() - 1);
		for (int i = 0; i < arguments->length(); ++i) {
			Ref<StringList, Owner> nameAndType = parts->at(i).split(":");
			String name, type;
			if (nameAndType->length() == 2) {
				name = nameAndType->at(0);
				type = nameAndType->at(1);
			}
			else
				type = parts->at(i);
			arguments->set(i, new Argument(name, type));
		}
		String returnType = parts->at(parts->length() - 1);
		type = new Type(displayString(returnType, arguments), returnType, arguments);
	}
	else if (parts->at(0)->size() != 0) {
		type = new Type(displayString(typeString), typeString);
	}
	return type;
}

Ref<Tip, Owner> HaxeMessageSyntax::parse(String message)
{
	Ref<Tip, Owner> tip;
	Ref<Token, Owner> rootToken = match(message);
	if (rootToken) {
		Ref<Token> token = rootToken->firstChild();
		if (token->rule() == typeTip_) {
			token = token->firstChild();
			if (token->rule() == value_) {
				String typeString = readValue(message, token);
				tip = new TypeTip(readType(typeString));
			}
		}
		else if (token->rule() == membersTip_) {
			Ref<Members, Owner> members = new Members(token->countChildren());
			int memberIndex = 0;
			token = token->firstChild();
			while (token) {
				String name, description;
				Ref<Type, Owner> type;
				Ref<Token> child = token->firstChild();
				while (child) {
					if (child->rule() == memberName_)
						name = message->copy(child->i0(), child->i1());
					else if (child->rule() == memberType_)
						type = readType(readValue(message, child));
					else if (child->rule() == memberDescription_)
						description = message->copy(child->i0(), child->i1());;
					child = child->nextSibling();
				}
				// debug("\"%%\",\"%%\",\"%%\"\n", name, type, description);
				members->set(memberIndex++, new Member(name, type, description));
				token = token->nextSibling();
			}
			tip = new MembersTip(members);
		}
	}
	
	return tip;
}

} // namespace codetips
