charcoal.syntax["neko"] = function()
{
	DEFINE("Comment",
		CHOICE(
			GLUE(
				STRING("/*"),
				FIND(STRING("*/"))
			),
			GLUE(
				STRING("//"),
				FIND(AHEAD(CHAR('\n')))
			)
		)
	);
	
	DEFINE("Identifier",
		GLUE(
			CHOICE(
				RANGE('a', 'z'),
				RANGE('A', 'Z'),
				RANGE("_@")
			),
			REPEAT(
				CHOICE(
					RANGE('a', 'z'),
					RANGE('A', 'Z'),
					RANGE('0', '9'),
					RANGE("_@")
				)
			)
		)
	);
	
	DEFINE("Integer",
		CHOICE(
			GLUE(
				STRING("0x"),
				REPEAT(1,
					CHOICE(
						RANGE('0', '9'),
						RANGE('a', 'f'),
						RANGE('A', 'F')
					)
				)
			),
			REPEAT(1, RANGE('0', '9'))
		)
	);
	
	DEFINE("Float",
		CHOICE(
			GLUE(
				REPEAT(1, RANGE('0', '9')),
				CHAR('.'),
				REPEAT(RANGE('0', '9'))
			),
			GLUE(
				CHAR('.'),
				REPEAT(1, RANGE('0', '9'))
			)
		)
	);
	
	DEFINE("Boolean",
		KEYWORD("true false")
	);
	
	DEFINE("EscapedChar",
		GLUE(
			CHAR('\\'),
			CHOICE(
				REPEAT(1, 3, RANGE('0', '9')),
				OTHER('\n')
			)
		)
	);
	
	DEFINE("String",
		GLUE(
			CHAR('"'),
			REPEAT(
				CHOICE(
					REF("EscapedChar"),
					OTHER('\"')
				)
			),
			CHAR('"')
		)
	);
	
	DEFINE("Builtin",
		CHOICE(
			GLUE(
				CHAR('$'),
				INLINE("Identifier")
			),
			KEYWORD("null this")
		)
	);
	
	DEFINE("Operator",
		REPEAT(1, RANGE("!=*/<>&|^%+:-"))
	);
	
	DEFINE("Keyword",
		KEYWORD("var while do if else try catch function return break continue switch default")
	);
	
	DEFINE("NekoSource",
		REPEAT(
			FIND(
				CHOICE(
					RANGE(" \n\t\r"),
					REF("Comment"),
					REF("Float"),
					REF("Integer"),
					REF("Boolean"),
					REF("String"),
					REF("Builtin"),
					REF("Operator"),
					REF("Keyword"),
					REF("Identifier")
				)
			)
		)
	);
	
	ENTRY("NekoSource");
}
