charcoal.syntax["tex"] = function()
{
	DEFINE("Comment",
		GLUE(
			STRING("%"),
			FIND(AHEAD(CHAR('\n')))
		)
	);
	
	DEFINE("Identifier",
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
	
	DEFINE("Command",
		GLUE(
			CHAR('\\'),
			REPEAT(0, 1, CHAR('@')),
			INLINE("Identifier"),
			REPEAT(0, 1,
				GLUE(
					REPEAT(RANGE(" \t")),
					CHAR('['),
					REPEAT(
						CHOICE(
							REF("Identifier"),
							REF("Number"),
							OTHER(']')
						)
					),
					CHAR(']')
				)
			)
		)
	);
	
	DEFINE("EscapedChar",
		GLUE(
			CHAR('\\'),
			ANY()
		)
	);
	
	DEFINE_VOID("HexDigit",
		CHOICE(
			RANGE('0', '9'),
			RANGE('a', 'f'),
			RANGE('A', 'F')
		)
	);
	
	DEFINE("Integer",
		CHOICE(
			GLUE(
				STRING("0x"),
				REPEAT(1, INLINE("HexDigit"))
			),
			GLUE(CHAR('0'), REPEAT(1, RANGE('0', '7'))),
			REPEAT(1, RANGE('0', '9'))
		)
	);
	
	DEFINE("Float",
		GLUE(
			CHOICE(
				GLUE(
					REPEAT(1, RANGE('0', '9')),
					CHOICE(
						AHEAD(RANGE("eE")),
						GLUE(
							CHAR('.'),
							REPEAT(RANGE('0', '9'))
						)
					)
				),
				GLUE(
					REPEAT(RANGE('0', '9')),
					CHAR('.'),
					REPEAT(1, RANGE('0', '9'))
				)
			),
			REPEAT(0, 1,
				GLUE(
					RANGE("eE"),
					REPEAT(0, 1, RANGE("+-")),
					REPEAT(1, RANGE('0', '9'))
				)
			)
		)
	);
	
	DEFINE("Number",
		GLUE(
			REPEAT(0, 1, RANGE("+-")),
			CHOICE(
				INLINE("Float"),
				INLINE("Integer")
			),
			REPEAT(0, 1,
				GLUE(
					REPEAT(RANGE(" \t")),
					KEYWORD("in cm mm pt")
				)
			)
		)
	);
	
	DEFINE("Argument",
		GLUE(
			CHAR('#'),
			INLINE("Integer")
		)
	);
	
	DEFINE("TexSource",
		REPEAT(
			FIND(
				CHOICE(
					REPEAT(1, RANGE(" \t\r")),
					REF("Comment"),
					REF("Command"),
					REF("Argument"),
					REF("EscapedChar")
				)
			)
		)
	);
	
	ENTRY("TexSource");
}
