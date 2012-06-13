charcoal.syntax["as3"] = function()
{
	DEFINE_VOID("Whitespace",
		REPEAT(1, RANGE(" \t\r"))
	);
	
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
	
	DEFINE("Punctuator",
		KEYWORD("\
			+ - * % ++ -- \
			& ~ | ^ << >> >>> \
			== < > <= >= != === !== \
			&& ! || \
			= \
			+= -= *= %= \
			&= |= ^= <<= >>= >>>= \
			&&= ||= \
			? @ [ ] { } ( ) ; : , . \
		")
	);
	
	DEFINE("DivPunctuator",
		GLUE(
			NOT(
				CHOICE(
					PREVIOUS("Punctuator"),
					PREVIOUS("DivPunctuator")
				)
			),
			KEYWORD(
				"/   /="
			)
		)
	);
	
	DEFINE("Keyword",
		GLUE(
			KEYWORD("\
				break case continue default do while else for in each \
				if label return super switch throw try catch finally \
				while with dynamic final internal native override private \
				protected public static class const extends function \
				get implements interface namespace package set var \
				import include use null this new \
				"
			),
			NOT(INLINE("Identifier"))
		)
	);
	
	DEFINE("NullLiteral",
		KEYWORD("null")
	);
	
	DEFINE("BooleanLiteral",
		KEYWORD("true false")
	);
	
	DEFINE_VOID("HexDigit",
		CHOICE(
			RANGE('0', '9'),
			RANGE('a', 'f'),
			RANGE('A', 'F')
		)
	);
	
	DEFINE("EscapeSequence",
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
	
	DEFINE("StringLiteral",
		CHOICE(
			GLUE(
				CHAR('"'),
				REPEAT(
					CHOICE(
						REF("EscapeSequence"),
						EXCEPT("\"\n")
					)
				),
				CHAR('"')
			),
			GLUE(
				CHAR('\''),
				REPEAT(
					CHOICE(
						REF("EscapeSequence"),
						EXCEPT("\'\n")
					)
				),
				CHAR('\'')
			)
		)
	);
	
	DEFINE("DecimalLiteral",
		GLUE(
			CHOICE(
				GLUE(
					REPEAT(1, RANGE('0', '9')),
					REPEAT(0, 1, CHAR('.')),
					REPEAT(RANGE('0', '9'))
				),
				GLUE(
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
	
	DEFINE("HexIntegerLiteral",
		GLUE(
			CHAR('0'),
			RANGE("xX"),
			REPEAT(1, INLINE("HexDigit"))
		)
	);
	
	DEFINE("NumericLiteral",
		CHOICE(
			REF("HexIntegerLiteral"),
			REF("DecimalLiteral")
		)
	);
	
	DEFINE("BackslashSequence",
		GLUE(
			CHAR('\\'),
			OTHER('\n')
		)
	);
	
	DEFINE("RegularExpressionLiteral",
		GLUE(
			CHAR('/'),
			EXCEPT("\n*/\\"),
			REPEAT(
				CHOICE(
					EXCEPT("\n/\\"),
					REF("BackslashSequence")
				)
			),
			CHAR('/'),
			REPEAT(0, 4, RANGE("gimy"))
		)
	);
	
	DEFINE("Identifier",
		GLUE(
			NOT(
				INLINE("ReservedWord")
			),
			GLUE(
				CHOICE(
					RANGE('A', 'Z'),
					RANGE('a', 'z'),
					RANGE("_$")
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
		)
	);
	
	DEFINE("ReservedWord",
		GLUE(
			CHOICE(
				REF("Keyword"),
				REF("NullLiteral"),
				REF("BooleanLiteral")
			),
			NOT(
				CHOICE(
					RANGE('a', 'z'),
					RANGE('A', 'Z'),
					CHAR('_'),
					RANGE('0', '9')
				)
			)
		)
	);
	
	DEFINE("As3Source",
		REPEAT(
			FIND(
				CHOICE(
					INLINE("Whitespace"),
					REF("Comment"),
					REF("ReservedWord"),
					REF("Identifier"),
					REF("NumericLiteral"),
					REF("Punctuator"),
					REF("DivPunctuator"),
					REF("RegularExpressionLiteral"),
					REF("StringLiteral")
				)
			)
		)
	);
	
	ENTRY("As3Source");
}
