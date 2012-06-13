charcoal.syntax["ruby"] = function()
{
	STATE_FLAG("doubleQuote", false);
	STATE_FLAG("bracketedString", false);
	STATE_FLAG("regex", false);
	STATE_CHAR("quotationMark", '\0');
	STATE_CHAR("quotationMarkOpening", '\0');
	STATE_STRING("endOfDocument", "");
	
	DEFINE_VOID("Whitespace",
		REPEAT(1, RANGE(" \t\r"))
	);
	
	DEFINE("Comment",
		GLUE(
			CHAR('#'),
			FIND(
				CHOICE(
					AHEAD(CHAR('\n')),
					EOI()
				)
			)
		)
	);
	
	DEFINE("BlockComment",
		GLUE(
			STRING("=begin"),
			FIND(
				GLUE(
					CHAR('\n'),
					STRING("=end")
				)
			)
		)
	);
	
	DEFINE_VOID("NameChar",
		CHOICE(
			RANGE('a', 'z'),
			RANGE('A', 'Z'),
			RANGE('0', '9'),
			RANGE("?_")
		)
	);
	
	DEFINE("Keyword",
		GLUE(
			// NOT(PREVIOUS("Operator")),
			KEYWORD(
				// "__LINE__ __FILE__ "
				"BEGIN END " +
				"and " + // alias
				"begin break " +
				"case " + // class
				"do " + // def, defined?
				"else elsif end ensure " +
				"for " + // false
				"if in " +
				"next not " + // module, nil
				"or " +
				"redo rescue retry return " +
				"super " + // self
				"then " + // true
				"undef unless until " +
				"when while " +
				"yield " +
				// from object.c:
				"new private protected public " +
				"require "
			),
			NOT(INLINE("NameChar"))
		)
	);
	
	DEFINE("DeclarationKeyword",
		GLUE(
			NOT(PREVIOUS("Operator")),
			KEYWORD("class def alias module include"),
			NOT(INLINE("NameChar"))
		)
	);
	
	DEFINE("Builtin",
		GLUE(
			KEYWORD("__LINE__ __FILE__ true false nil defined? self"),
			NOT(INLINE("NameChar"))
		)
	);
	
	DEFINE("Operator",
		KEYWORD(
			"!= !~ " +
			"=== == = =~ => " +
			"<<= << <=> <= < " +
			">>= >> >= > " +
			"&&= &= & " +
			"||= |= | " +
			"**= *= * " +
			"+= + " +
			"-= - " +
			"/= / " +
			"%= % " +
			"... .. . " +
			"^= ^ " +
			"~ " +
			"[ ] " +
			":: ? : , ; "
		)
	);
	
	DEFINE("OpeningBracket", RANGE("(["));
	DEFINE("ClosingBracket", RANGE(")]"));
	
	DEFINE_VOID("KeyCode",
		GLUE(
			CHOICE(
				STRING("\\M-\\C-"),
				STRING("\\C-\\M-"),
				STRING("\\M-"),
				STRING("\\C-"),
				PASS()
			),
			RANGE(33, 127)
		)
	);
	
	DEFINE("Integer",
		GLUE(
			CHOICE(
				GLUE(
					RANGE('1', '9'),
					REPEAT(
						GLUE(
							REPEAT(0, 1, CHAR('_')),
							RANGE('0', '9')
						)
					)
				),
				GLUE(
					CHAR('0'),
					REPEAT(0, 1, RANGE("oO")),
					RANGE('0', '7'),
					REPEAT(
						GLUE(
							REPEAT(0, 1, CHAR('_')),
							RANGE('0', '7')
						)
					)
				),
				GLUE(
					CHAR('0'),
					RANGE("xX"),
					CHOICE(
						RANGE('0', '9'),
						RANGE('A', 'F'),
						RANGE('a', 'f')
					),
					REPEAT(
						GLUE(
							REPEAT(0, 1, CHAR('_')),
							CHOICE(
								RANGE('0', '9'),
								RANGE('A', 'F'),
								RANGE('a', 'f')
							)
						)
					)
				),
				GLUE(
					CHAR('0'),
					RANGE("bB"),
					RANGE('0', '1'),
					REPEAT(
						GLUE(
							REPEAT(0, 1, CHAR('_')),
							RANGE('0', '1')
						)
					)
				),
				CHAR('0'),
				GLUE(
					CHAR('?'),
					INLINE("KeyCode")
				)
			),
			NOT(
				CHOICE(
					RANGE('a', 'z'),
					RANGE('A', 'Z'),
					CHAR('_')
				)
			)
		)
	);
	
	DEFINE("Float",
		GLUE(
			REPEAT(1, RANGE('0', '9')),
			CHOICE(
				GLUE(
					CHAR('.'),
					NOT(INLINE("LocalName")),
					REPEAT(RANGE('0', '9')),
					REPEAT(0, 1,
						GLUE(
							RANGE("eE"),
							REPEAT(0, 1, RANGE("+-")),
							REPEAT(1, RANGE('0', '9'))
						)
					)
				),
				GLUE(
					RANGE("eE"),
					REPEAT(0, 1, RANGE("+-")),
					REPEAT(1, RANGE('0', '9')),
					REPEAT(0, 1, RANGE("jJ"))
				)
			),
			NOT(CHAR('.'))
		)
	);
	
	DEFINE("EscapeSequence",
		GLUE(
			CHAR('\\'),
			IF("doubleQuote",
				CHOICE(
					REPEAT(1, 3, RANGE('0', '7')),
					GLUE(
						CHAR('x'),
						REPEAT(1, 2,
							CHOICE(
								RANGE('0', '9'),
								RANGE('A', 'F'),
								RANGE('a', 'f')
							)
						)
					),
					GLUE(
						CHAR('c'),
						ANY()
					),
					INLINE("KeyCode"),
					ANY()
				),
				RANGE("\'\\")
			)
		)
	);
	
	DEFINE_VOID("StringBody",
		REPEAT(
			CHOICE(
				IF("doubleQuote",
					REF("Interpolation"),
					FAIL()
				),
				REF("EscapeSequence"),
				IF("bracketedString",
					GLUE(
						VARCHAR("quotationMarkOpening"),
						INLINE("StringBody"),
						VARCHAR("quotationMark")
					),
					FAIL()
				),
				VAROTHER("quotationMark")
			)
		)
	);
	
	DEFINE("String",
		CHOICE(
			GLUE(
				CHAR('?'),
				CHOICE(
					REF("EscapeSequence"),
					EXCEPT(" \t\n")
				)
			),
			GLUE(
				CHOICE(
					GLUE(
						CHAR('%'),
						NOT(
							CHOICE(
								PREVIOUS("Integer"),
								PREVIOUS("Float"),
								PREVIOUS("String"),
								PREVIOUS("ClosingBracket"),
								PREVIOUS("DeclarationKeyword"),
								PREVIOUS("Builtin")
							)
						),
						CHOICE(
							GLUE(
								RANGE("qw"),
								SET("doubleQuote", false),
								SET("regex", false)
							),
							GLUE(
								RANGE("QWx"),
								SET("doubleQuote", true),
								SET("regex", false)
							),
							GLUE(
								RANGE("r"),
								SET("doubleQuote", true),
								SET("regex", true)
							),
							GLUE(
								SET("doubleQuote", true),
								SET("regex", false)
							)
						),
						CHOICE(
							GLUE(
								AHEAD(
									CHOICE(
										RANGE("'\"!$%&/\\?`#+-*;:|~^@"),
										GLUE(
											NOT(PREVIOUS("LocalName")),
											CHAR('=')
										)
									)
								),
								GETCHAR("quotationMark")
							),
							GLUE(
								CHOICE(
									GLUE(CHAR('['), SETCHAR("quotationMarkOpening", '['), SETCHAR("quotationMark", ']')),
									GLUE(CHAR('('), SETCHAR("quotationMarkOpening", '('), SETCHAR("quotationMark", ')')),
									GLUE(CHAR('{'), SETCHAR("quotationMarkOpening", '{'), SETCHAR("quotationMark", '}')),
									GLUE(CHAR('<'), SETCHAR("quotationMarkOpening", '<'), SETCHAR("quotationMark", '>'))
								),
								SET("bracketedString", true)
							)
						)
					),
					GLUE(
						AHEAD(RANGE("\"`")),
						SET("doubleQuote", true),
						SET("regex", false),
						GETCHAR("quotationMark")
					),
					GLUE(
						CHAR('\''),
						SET("doubleQuote", false),
						SET("regex", false),
						SETCHAR("quotationMark", '\'')
					),
					GLUE(
						NOT(
							CHOICE(
								PREVIOUS("Integer"),
								PREVIOUS("Float"),
								PREVIOUS("String"),
								PREVIOUS("LocalName"),
								PREVIOUS("MemberName"),
								PREVIOUS("ConstantName"),
								PREVIOUS("ClassMemberName"),
								PREVIOUS("GlobalName"),
								PREVIOUS("ClosingBracket"),
								PREVIOUS("DeclarationKeyword"),
								PREVIOUS("Builtin")
							)
						),
						CHAR('/'),
						SET("doubleQuote", true),
						SET("regex", true),
						SETCHAR("quotationMark", '/')
					)
				),
				INLINE("StringBody"),
				VARCHAR("quotationMark"),
				IF("regex",
					REPEAT(RANGE("emxuiosn")),
					PASS()
				),
				SET("doubleQuote", false),
				SET("bracketedString", false),
				SET("regex", false),
				SETCHAR("quotationMark", '\0'),
				SETCHAR("quotationMarkOpening", '\0')
			)
		)
	);
	
	DEFINE("Document",
		GLUE(
			NOT(
				CHOICE(
					PREVIOUS("Integer"),
					PREVIOUS("Float"),
					PREVIOUS("String"),
					// PREVIOUS("LocalName"),
					PREVIOUS("ClosingBracket"),
					PREVIOUS("DeclarationKeyword")
				)
			),
			STRING("<<"),
			REPEAT(0, 1, CHAR('-')),
			NOT(INLINE("Whitespace")), // to distinguish from shift left
			GETSTRING("endOfDocument", REPEAT(1, INLINE("NameChar"))),
			REPEAT(INLINE("Whitespace")),
			SET("doubleQuote", true),
			REPEAT(
				CHOICE(
					REF("Interpolation"),
					REF("EscapeSequence"),
					GLUE(
						NOT(
							GLUE(
								CHAR('\n'),
								REPEAT(0, 1, INLINE("Whitespace")),
								VARSTRING("endOfDocument")
							)
						),
						ANY()
					)
				)
			),
			CHAR('\n'),
			REPEAT(0, 1, INLINE("Whitespace")),
			VARSTRING("endOfDocument"),
			SET("doubleQuote", false)
		)
	);
	
	DEFINE("ParentModuleName", INLINE("ClassName"));
	
	DEFINE("ModuleName",
		GLUE(
			CHOICE(
				PREVIOUS("DeclarationKeyword", "module"),
				PREVIOUS("DeclarationKeyword", "include")
			),
			REPEAT(
				GLUE(
					REF("ParentModuleName"),
					AHEAD(STRING("::")),
					REF("Operator")
				)
			),
			INLINE("ClassName")
		)
	);
	
	DEFINE("ClassDeclarationName",
		GLUE(
			PREVIOUS("DeclarationKeyword", "class"),
			INLINE("ClassName")
		)
	);
	
	DEFINE("MethodDeclarationName",
		GLUE(
			PREVIOUS("DeclarationKeyword", "def"),
			NOT(INLINE("Builtin")), /* e.g. "self" */
			INLINE("LocalName")
		)
	);
	
	DEFINE("ClassMethodDeclarationName",
		GLUE(
			PREVIOUS("DeclarationKeyword", "def"),
			CHOICE(
				REF("ClassName"),
				REF("Builtin")
			),
			AHEAD(CHAR('.')),
			REF("Operator"),
			INLINE("LocalName")
		)
	);
	
	DEFINE("OperatorDeclarationName",
		GLUE(
			PREVIOUS("DeclarationKeyword", "def"),
			KEYWORD("\
				! ~ +@ \
				** \
				-@ \
				* / % \
				+ - \
				<< >> \
				& \
				| ^ \
				< <= >= > \
				== === != =~ !~ <=> \
			")
		)
	);
	
	DEFINE("ClassName",
		GLUE(
			RANGE('A', 'Z'),
			REPEAT(1,
				CHOICE(
					RANGE('a', 'z'),
					RANGE('A', 'Z'),
					CHAR('_'),
					RANGE('0', '9')
				)
			)
		)
	);
	
	DEFINE("ConstantName",
		GLUE(
			RANGE('A', 'Z'),
			REPEAT(1,
				CHOICE(
					RANGE('A', 'Z'),
					CHAR('_'),
					RANGE('0', '9')
				)
			)
		)
	);
	
	DEFINE("GlobalName",
		GLUE(
			CHAR('$'),
			CHOICE(
				RANGE("!@;,><+_.&~=/\\*$?'`\""),
				RANGE('0', '9'),
				INLINE("LocalName")
			)
		)
	);
	
	DEFINE("ClassMemberName",
		GLUE(
			STRING("@@"),
			INLINE("LocalName")
		)
	);
	
	DEFINE("MemberName",
		GLUE(
			CHAR('@'),
			INLINE("LocalName")
		)
	);

	DEFINE("LocalName",
		GLUE(
			CHOICE(
				RANGE('a', 'z'),
				CHAR('_')
			),
			REPEAT(
				CHOICE(
					RANGE('a', 'z'),
					CHAR('_'),
					RANGE('A', 'Z'),
					RANGE('0', '9')
				)
			),
			REPEAT(0, 1, RANGE("!?")) // = intermediate HACK
		)
	);
	
	DEFINE("Symbol",
		GLUE(
			CHAR(':'),
			REPEAT(1,
				CHOICE(
					RANGE('a', 'z'),
					RANGE('A', 'Z'),
					RANGE('0', '9'),
					CHAR('_')
				)
			),
			REPEAT(0, 1, RANGE("!?")) // = intermediate HACK
			// QUICK HACK, missing :s[smth] notation!
		)
	);
	
	DEFINE("Interpolation",
		GLUE(
			CHAR('#'),
			REF("Block")
		)
	);
	
	DEFINE("Block",
		GLUE(
			CHAR('{'),
			INVOKE("ruby"),
			CHAR('}')
		)
	);
	
	DEFINE("RubySource",
		REPEAT(
			GLUE(
				NOT(CHAR('}')),
				FIND(
					CHOICE(
						INLINE("Whitespace"),
						REF("Comment"),
						GLUE(
							CHOICE(
								BOI(),
								CHAR('\n')
							),
							REF("BlockComment")
						),
						REF("Block"),
						REF("Interpolation"),
						REF("String"),
						REF("Document"),
						REF("Keyword"),
						GLUE(
							REF("DeclarationKeyword"),
							INLINE("Whitespace"),
							REPEAT(0, 1,
								CHOICE(
									REF("ModuleName"),
									REF("ClassDeclarationName"),
									REF("MethodDeclarationName"),
									REF("ClassMethodDeclarationName"),
									REF("OperatorDeclarationName")
								)
							)
						),
						REF("Builtin"),
						REF("LocalName"),
						REF("MemberName"),
						REF("ClassMemberName"),
						REF("ClassName"),
						REF("ConstantName"),
						REF("Symbol"),
						REF("OpeningBracket"),
						REF("ClosingBracket"),
						REF("Float"),
						REF("Integer"),
						REF("Operator"),
						REF("GlobalName")
					)
				)
			)
		)
	);
	
	ENTRY("RubySource");
}
