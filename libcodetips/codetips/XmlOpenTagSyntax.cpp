/*
 * XmlOpenTagSyntax.cpp -- syntax of an opening XML tag without closing '>'
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */

#include "XmlOpenTagSyntax.hpp"

namespace codetips
{

XmlOpenTagSyntax::XmlOpenTagSyntax()
{
	DEFINE_VOID("Ws",
		REPEAT(
			RANGE(" \t")
		)
	);
	
	DEFINE("Name",
		GLUE(
			CHOICE(
				RANGE('a', 'z'),
				RANGE('A', 'Z'),
				CHAR('_')
			),
			REPEAT(
				CHOICE(
					RANGE('a', 'z'),
					RANGE('A', 'Z'),
					RANGE('0', '9'),
					CHAR('_')
				)
			)
		)
	);
	
	DEFINE("Value",
		CHOICE(
			GLUE(
				CHAR('"'),
				FIND(CHAR('"'))
			),
			GLUE(
				CHAR('\''),
				FIND(CHAR('\''))
			)
		)
	);
	
	DEFINE("Attribute",
		GLUE(
			REF("Name"),
			REF("Ws"),
			CHAR('='),
			REF("Ws"),
			REF("Value")
		)
	);
	
	DEFINE("OpenTag",
		GLUE(
			CHAR('<'),
			REF("Ws"),
			REF("Name"),
			REPEAT(
				GLUE(
					REF("Ws"),
					REF("Attribute")
				)
			),
			REF("Ws")
		)
	);
	
	ENTRY("OpenTag");
}

} // namespace codetips
