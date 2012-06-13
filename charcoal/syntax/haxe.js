charcoal.syntax["haxe"] = function()
{
	STATE_CHAR("quotation", '\0');
	
	DEFINE("BlockComment",
		GLUE(
			STRING("/*"),
			FIND(STRING("*/"))
		)
	);
	
	DEFINE("LineComment",
		GLUE(
			STRING("//"),
			FIND(AHEAD(CHAR('\n')))
		)
	);
	
	DEFINE("DocComment",
		CHOICE(
			GLUE(
				AHEAD(STRING("/**")),
				REF("BlockComment")
			),
			GLUE(
				AHEAD(STRING("//!")),
				REF("LineComment")
			)
		)
	);
	
	DEFINE("Comment",
		CHOICE(
			REF("DocComment"),
			REF("BlockComment"),
			REF("LineComment")
		)
	);
	
	DEFINE("Keyword",
		GLUE(
			KEYWORD("\
				function class static var if else while do for \
				break return continue extends implements import \
				switch case default public private try untyped \
				catch new this throw extern enum in interface \
				cast override dynamic typedef package callback \
				inline using super"
			),
			NOT(INLINE("Identifier"))
		)
	);
	
	DEFINE("MacroConditional",
		GLUE(
			REPEAT(0, 1, CHAR('!')),
			CHOICE(
				GLUE(
					CHAR('('),
					FIND(CHAR(')'))
				),
				REF("SimpleIdentifier")
			)
		)
	);
	
	DEFINE("Macro",
		GLUE(
			CHAR('#'),
			CHOICE(
				GLUE(
					KEYWORD("if elseif"),
					REPEAT(RANGE(" \t")),
					REF("MacroConditional")
				),
				KEYWORD("else end error line")
			),
			NOT(INLINE("SimpleIdentifier"))
		)
	);
	
	DEFINE("TypeIdentifier",
		GLUE(
			REPEAT(
				GLUE(
					REF("Identifier"),
					CHAR('.')
				)
			),
			// AHEAD(RANGE('A', 'Z')),
			REF("Identifier")
		)
	);
	
	DEFINE_VOID("SimpleIdentifier",
		GLUE(
			NOT(INLINE("Keyword")),
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
	
	DEFINE("BuiltInIdentifier",
		GLUE(
			KEYWORD(
				"Void Float Int UInt Null Bool Dynamic    \
				 Iterator Iterable ArrayAccess            \
				 Array Class Date Enum EReg               \
				 Hash IntHash IntIter Lambda List Match   \
				 Reflect String \
				 Type Xml"
			),
			NOT(
				CHOICE(
					RANGE('a', 'z'),
					RANGE('A', 'Z'),
					RANGE('0', '9'),
					CHAR('_')
				)
			)
		)
	);
	
	DEFINE("Identifier",
		GLUE(
			CHOICE(
				REF("BuiltInIdentifier"),
				INLINE("SimpleIdentifier")
			),
			REPEAT(
				GLUE(
					REPEAT(INLINE("Whitespace")),
					CHAR('<'),
					REPEAT(
						GLUE(
							NOT(CHAR('>')),
							CHOICE(
								REPEAT(1, INLINE("Whitespace")),
								REF("Identifier"),
								REF("Keyword"),
								REF("Macro"),
								RANGE("?:,"),
								STRING("->")
							)
						)
					),
					CHAR('>')
				)
			)
		)
	);
	
	DEFINE("Operator",
		KEYWORD("\
			++ -- ~ \
			%= &= |= ^= += -= *= /= <<= >>= >>>= \
			== != <= >= && || << -> ... \
			! < > ; : , . % & | ^ + * / - = \
			? @ [ ] \
		")  // ( )
	);
	
	DEFINE_VOID("HexDigit",
		CHOICE(
			RANGE('0', '9'),
			RANGE('a', 'f'),
			RANGE('A', 'F')
		)
	);
	
	DEFINE("Boolean",
		KEYWORD("true false")
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
			),
			NOT(CHAR('.')) // to distinguish from op '...'
		)
	);
	
	DEFINE("EscapedChar",
		GLUE(
			CHAR('\\'),
			CHOICE(
				RANGE("'\"\\bfnrtv\n"),
				GLUE(
					CHAR('0'),
					NOT(RANGE('0', '9'))
				),
				GLUE(
					CHAR('x'),
					REPEAT(2, 2, INLINE("HexDigit"))
				),
				GLUE(
					CHAR('u'),
					REPEAT(4, 4, INLINE("HexDigit"))
				)
			)
		)
	);
	
	DEFINE("String",
		GLUE(
			AHEAD(RANGE("\"'")),
			GETCHAR("quotation"),
			REPEAT(
				CHOICE(
					REF("EscapedChar"),
					VAROTHER("quotation")
				)
			),
			VARCHAR("quotation")
		)
	);
	
	DEFINE("AnyEscapedChar",
		GLUE(
			CHAR('\\'),
			OTHER('\n')
		)
	);
	
	DEFINE("RegExp",
		GLUE(
			STRING("~/"),
			REPEAT(
				CHOICE(
					EXCEPT("\n/\\"),
					REF("AnyEscapedChar")
				)
			),
			CHAR('/'),
			REPEAT(0, 4, RANGE("igms"))
		)
	);
	
	DEFINE_VOID("Block",
		GLUE(
			CHAR('{'),
			REPEAT(0, 1,
				REF("HaxeSource")
			),
			CHAR('}')
		)
	);
	
	DEFINE_VOID("Whitespace",
		RANGE(" \t\n\r")
	);
	
	DEFINE("FunctionIdentifier",
		CHOICE(
			INLINE("Identifier"),
			STRING("new")
		)
	);
	DEFINE("VariableIdentifier", INLINE("Identifier"));
	DEFINE("ClassIdentifier",    INLINE("Identifier"));
	DEFINE("EnumIdentifier",     INLINE("Identifier"));
	DEFINE("PackageIdentifier",  INLINE("TypeIdentifier"));
	
	DEFINE("Function",
		GLUE(
			PREVIOUS("Keyword", "function"),
			REF("FunctionIdentifier"),
			REPEAT(INLINE("Whitespace")),
			CHAR('('),
			REPEAT(
				GLUE(
					NOT(CHAR(')')),
					CHOICE(
						REPEAT(1, INLINE("Whitespace")),
						REF("Keyword"),
						REF("TypeIdentifier"),
						REF("Macro"),
						RANGE("?:,<->")
					)
				)
			),
			CHAR(')'),
			REPEAT(
				CHOICE(
					REPEAT(1, INLINE("Whitespace")),
					REF("Keyword"),
					REF("TypeIdentifier"),
					REF("Macro"),
					RANGE("?:,<->")
				)
			),
			CHOICE(
				INLINE("Block"),
				CHAR(';')
			)
		)
	);
	
	DEFINE("Variable",
		GLUE(
			CHOICE(
				GLUE(
					PREVIOUS("Keyword", "var"),
					REF("VariableIdentifier")
				),
				GLUE(
					PREVIOUS("Variable"),
					REF("VariableIdentifier")
				)
			),
			REPEAT(INLINE("Whitespace")),
			REPEAT(0, 1,
				GLUE(
					CHAR(':'),
					REPEAT(INLINE("Whitespace")),
					REF("TypeIdentifier")
				)
			),
			REPEAT(INLINE("Whitespace")),
			REPEAT(0, 1,
				GLUE(
					AHEAD(CHAR('=')),
					REPEAT(REF("Operator")),
					REPEAT(INLINE("Whitespace")),
					CHOICE(
						REF("Float"),
						REF("Integer"),
						REF("Boolean"),
						REF("String"),
						REF("Identifier")
					)
				)
			)
		)
	);
	
	DEFINE("Class",
		GLUE(
			PREVIOUS("Keyword", "class"),
			REF("ClassIdentifier"),
			REPEAT(
				CHOICE(
					REPEAT(1, INLINE("Whitespace")),
					REF("Keyword"),
					REF("TypeIdentifier"),
					REF("Macro")
				)
			),
			REPEAT(INLINE("Whitespace")),
			INLINE("Block")
		)
	);
	
	DEFINE("Enum",
		GLUE(
			PREVIOUS("Keyword", "enum"),
			REF("EnumIdentifier"),
			REPEAT(INLINE("Whitespace")),
			INLINE("Block")
		)
	);
	
	DEFINE("Import",
		GLUE(
			PREVIOUS("Keyword", "import"),
			REF("PackageIdentifier")
		)
	);
	
	DEFINE("HaxeSource",
		REPEAT(
			GLUE(
				NOT(CHAR('}')),
				FIND(
					CHOICE(
						REPEAT(1, INLINE("Whitespace")),
						REF("Comment"),
						REF("String"),
						REF("RegExp"),
						REF("Macro"),
						GLUE(
							REF("Keyword"),
							REPEAT(0, 1,
								GLUE(
									REPEAT(1, INLINE("Whitespace")),
									CHOICE(
										REF("Function"),
										GLUE(
											REF("Variable"),
											REPEAT(
												GLUE(
													REPEAT(INLINE("Whitespace")),
													CHAR(','),
													// REF("Operator"),
													REPEAT(INLINE("Whitespace")),
													REF("Variable")
												)
											)
										),
										REF("Class"),
										REF("Enum"),
										REF("Import")
									)
								)
							)
						),
						REF("TypeIdentifier"),
						REF("Identifier"),
						REF("Operator"),
						REF("Float"),
						REF("Integer"),
						REF("Boolean"),
						INLINE("Block")
					)
				)
			)
		)
	);
	
	DEFINE("Package",
		GLUE(
			REPEAT(INLINE("Whitespace")),
			REPEAT(0, 1, REF("Comment")),
			REPEAT(INLINE("Whitespace")),
			REPEAT(0, 1,
				FIND(
					GLUE(
						AHEAD(STRING("package")),
						REF("Keyword"),
						REPEAT(1, INLINE("Whitespace")),
						REF("PackageIdentifier")
					)
				)
			),
			REF("HaxeSource")
		)
	);
	
	ENTRY("Package");
}
