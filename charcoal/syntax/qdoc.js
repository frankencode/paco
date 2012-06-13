charcoal.syntax["qdoc"] = function()
{
	STATE_FLAG("verbatim", false);
	STATE_STRING("codeEnd", "endcode");
	
	DEFINE("Word",
		REPEAT(1,
			CHOICE(
				RANGE('a', 'z'),
				RANGE('A', 'Z'),
				CHAR('_')
			)
		)
	);
	
	DEFINE("Brackets",
		GLUE(
			CHAR('('),
			REPEAT(
				CHOICE(
					REF("Brackets"),
					OTHER(')')
				)
			),
			CHAR(')')
		)
		/*note: may contain C strings! */
	);
	
	DEFINE("OpeningOrClosingParenthesis", RANGE("{}"));
	
	DEFINE("Parenthesis",
		GLUE(
			AHEAD(CHAR('{')),
			REF("OpeningOrClosingParenthesis"),
			REPEAT(
				CHOICE(
					IF("verbatim",
						FAIL(),
						REF("Command")
					),
					INLINE("Parenthesis"),
					OTHER('}')
				)
			),
			AHEAD(CHAR('}')),
			REF("OpeningOrClosingParenthesis")
		)
	);
	
	DEFINE("Span",
		CHOICE(
			GLUE(
				REF("Word"),
				REPEAT(RANGE(' \t')),
				REF("Brackets")
			),
			REF("Word"),
			REF("Parenthesis")
		)
	);
	
	DEFINE("Format",
		GLUE(
			CHAR('\\'),
			REPEAT(0, 1,
				GLUE(
					AHEAD(CHAR('c')),
					SET("verbatim", true)
				)
			),
			KEYWORD("\
				c a i tt bold sub sup underline"
			),
			NOT(INLINE("Word")),
			REPEAT(0, 1,
				GLUE(
					REPEAT(RANGE(' \t')),
					REF("Span")
				)
			),
			SET("verbatim", false)
		)
	);
	
	DEFINE("Title",
		FIND(
			AHEAD(
				GLUE(
					REPEAT(RANGE(" \t")),
					CHAR('\n')
				)
			)
		)
	);
	
	DEFINE("Structure",
		GLUE(
			CHAR('\\'),
			KEYWORD("\
				part chapter section1 section2 section3 section4"
			),
			NOT(INLINE("Word")),
			REPEAT(RANGE(" \t")),
			REF("Title")
		)
	);
	
	DEFINE("VerbatimCodeBody",
		INVOKE("cxx",
			FIND(
				AHEAD(
					GLUE(
						REPEAT(RANGE(" \t")),
						CHAR('\\'),
						VARSTRING("codeEnd")
					)
				)
			)
		)
	);
	
	DEFINE("VerbatimCode",
		GLUE(
			CHAR('\\'),
			CHOICE(
				KEYWORD("code badcode"),
				GLUE(
					KEYWORD("oldcode"),
					REPEAT(RANGE(" \t")),
					SETSTRING("codeEnd", "newcode"),
					REF("VerbatimCodeBody"),
					SETSTRING("codeEnd", "endcode"),
					REPEAT(RANGE(" \t")),
					CHAR('\\'),
					KEYWORD("newcode")
				)
			),
			REPEAT(RANGE(" \t")),
			REF("VerbatimCodeBody"),
			REPEAT(RANGE(" \t")),
			CHAR('\\'),
			KEYWORD("endcode")
		)
	);
	
	DEFINE("Command",
		GLUE(
			AHEAD(CHAR('\\')),
			CHOICE(
				STRING('\\\\'),
				REF("Format"),
				REF("Structure"),
				REF("VerbatimCode")
			)
		)
	);
	
	DEFINE("Text",
		REPEAT(
			FIND(
				REF("Command")
			)
		)
	);
	
	ENTRY("Text");
}
