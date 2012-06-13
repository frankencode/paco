charcoal.syntax["python"] = function()
{
	STATE_FLAG("commentIsOpen", false);
	STATE_FLAG("stringIsOpen", false);
	STATE_FLAG("doubleQuote", false);
	STATE_FLAG("triplet", false);
	STATE_FLAG("unicode", false);
	STATE_FLAG("raw", false);
	
	DEFINE("Keyword",
		GLUE(
			KEYWORD(
				"and       del       from      not       while \
				 as        elif      global    or        with  \
				 assert    else      if        pass      yield \
				 break     except    import    print           \
				 class     exec      in        raise           \
				 continue  finally   is        return          \
				 def       for       lambda    try             "
			),
			NOT(INLINE("Identifier"))
		)
	);
	
	DEFINE("Operator",
		KEYWORD(
			"+       -       *       **      /       //      %  \
			 <<      >>      &       |       ^       ~          \
			 <       >       <=      >=      ==      !=      <> "
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
					CHAR('_'),
					RANGE('0', '9')
				)
			)
		)
	);
	
	DEFINE("Comment",
		GLUE(
			CHAR('#'),
			FIND(
				CHOICE(CHAR('\n'), EOI())
			)
		)
	);
	
	DEFINE("String",
		GLUE(
			IF("stringIsOpen",
				PASS(),
				GLUE(
					REPEAT(0, 1,
						GLUE(
							RANGE("uU"),
							SET("unicode", true)
						)
					),
					REPEAT(0, 1,
						GLUE(
							RANGE("rR"),
							SET("raw", true)
						)
					),
					CHOICE(
						GLUE(
							STRING("\"\"\""),
							SET("doubleQuote", true),
							SET("triplet", true)
						),
						GLUE(
							CHAR('"'),
							SET("doubleQuote", true),
							SET("triplet", false)
						),
						GLUE(
							STRING("'''"),
							SET("doubleQuote", false),
							SET("triplet", true)
						),
						GLUE(
							CHAR('\''),
							SET("doubleQuote", false),
							SET("triplet", false)
						)
					)
				)
			),
			REPEAT(
				CHOICE(
					REF("EscapedChar"),
					GLUE(
						NOT(
							CHOICE(
								IF("doubleQuote",
									IF("triplet",
										STRING("\"\"\""),
										CHAR('\"')
									),
									IF("triplet",
										STRING("'''"),
										CHAR('\'')
									)
								),
								CHAR('\n')
							)
						),
						ANY()
					)
				)
			),
			CHOICE(
				GLUE(
					IF("doubleQuote",
						IF("triplet",
							STRING("\"\"\""),
							CHAR('\"')
						),
						IF("triplet",
							STRING("'''"),
							CHAR('\'')
						)
					),
					SET("stringIsOpen", false),
					SET("doubleQuote", false),
					SET("triplet", false),
					SET("unicode", false),
					SET("raw", false)
				),
				GLUE(
					CHOICE(CHAR('\n'), EOI()),
					SET("stringIsOpen", true)
				)
			)
		)
	);
	
	DEFINE("EscapedChar",
		GLUE(
			CHAR('\\'),
			CHOICE(
				IF("unicode",
					CHOICE(
						GLUE(
							CHAR('N'),
							CHAR('{'),
							REF("Identifier"),
							CHAR('}')
						),
						GLUE(
							CHAR('u'),
							REPEAT(4, 4,
								CHOICE(
									RANGE('0', '9'),
									RANGE('A', 'F'),
									RANGE('a', 'f')
								)
							)
						),
						GLUE(
							CHAR('U'),
							REPEAT(8, 8,
								CHOICE(
									RANGE('0', '9'),
									RANGE('A', 'F'),
									RANGE('a', 'f')
								)
							)
						)
					),
					FAIL()
				),
				IF("raw",
					FAIL(),
					CHOICE(
						RANGE("\n\\'\"abfnrtv"),
						GLUE(
							CHAR('x'),
							REPEAT(2, 2,
								CHOICE(
									RANGE('0', '9'),
									RANGE('A', 'F'),
									RANGE('a', 'f')
								)
							)
						),
						REPEAT(3, 3, RANGE('0', '7'))
					)
				)
			)
		)
	);
	
	DEFINE("Integer",
		GLUE(
			CHOICE(
				GLUE(
					RANGE('1', '9'),
					REPEAT(RANGE('0', '9'))
				),
				GLUE(
					CHAR('0'),
					REPEAT(0, 1, RANGE("oO")),
					REPEAT(1, RANGE('0', '7'))
				),
				GLUE(
					CHAR('0'),
					RANGE("xX"),
					REPEAT(1,
						CHOICE(
							RANGE('0', '9'),
							RANGE('A', 'F'),
							RANGE('a', 'f')
						)
					)
				),
				GLUE(
					CHAR('0'),
					RANGE("bB"),
					REPEAT(1, RANGE('0', '1'))
				),
				CHAR('0')
			),
			REPEAT(0, 1, RANGE("lL"))
		)
	);
	
	DEFINE("Float",
		GLUE(
			REPEAT(1, RANGE('0', '9')),
			CHOICE(
				GLUE(
					CHAR('.'),
					REPEAT(RANGE('0', '9')),
					REPEAT(0, 1,
						GLUE(
							RANGE("eE"),
							REPEAT(0, 1, RANGE("+-")),
							REPEAT(1, RANGE('0', '9'))
						)
					),
					REPEAT(0, 1, RANGE("jJ"))
				),
				GLUE(
					RANGE("eE"),
					REPEAT(0, 1, RANGE("+-")),
					REPEAT(1, RANGE('0', '9')),
					REPEAT(0, 1, RANGE("jJ"))
				),
				RANGE("jJ")
			),
			NOT(CHAR('.'))
		)
	);
	
	DEFINE("PythonSource",
		REPEAT(
			GLUE(
				NOT(EOI()),
				FIND(
					CHOICE(
						REPEAT(1, RANGE(" \t\r\n")),
						REF("Comment"),
						REF("String"),
						REF("Keyword"),
						REF("Identifier"),
						REF("Operator"),
						REF("Float"),
						REF("Integer")
					)
				)
			)
		)
	);
	
	ENTRY("PythonSource");
}
