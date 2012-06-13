charcoal.syntax["pascal"] = function()
{
	OPTION("CaseSensitive", false);
	
	DEFINE("Preprocessing",
		GLUE(
			STRING('{$'),
			FIND(CHAR('}'))
		)
	);
	
	DEFINE("Comment",
		CHOICE(
			GLUE(
				CHAR('{'),
				FIND(CHAR('}'))
			),
			GLUE(
				STRING("(*"),
				FIND(STRING("*)"))
			),
			GLUE(
				STRING("//"),
				FIND(
					CHOICE(
						CHAR('\n'),
						EOI()
					)
				)
			),
			GLUE(
				PREVIOUS("Done"),
				FIND(EOI())
			)
		)
	);
	
	DEFINE("QuotedString",
		GLUE(
			CHAR('\''),
			REPEAT(
				CHOICE(
					EXCEPT("'\n"),
					STRING("''")
				)
			),
			CHAR('\'')
		)
	);
	
	DEFINE("ControlString",
		GLUE(
			CHAR('#'),
			INLINE("UnsignedInteger")
		)
	);
	
	DEFINE("String",
		REPEAT(1,
			CHOICE(
				REF("QuotedString"),
				REF("ControlString")
			)
		)
	);
	
	DEFINE_VOID("UnsignedInteger",
		CHOICE(
			REPEAT(1, RANGE('0', '9')),
			GLUE(
				CHAR('$'),
				REPEAT(1,
					CHOICE(
						RANGE('0', '9'),
						RANGE('A', 'F'),
						RANGE('a', 'f')
					)
				)
			),
			GLUE(
				CHAR('&'),
				REPEAT(1, RANGE('0', '7'))
			),
			GLUE(
				CHAR('%'),
				REPEAT(1, RANGE("01"))
			)
		)
	);
	
	DEFINE("Integer",
		GLUE(
			REPEAT(0, 1, RANGE("+-")),
			INLINE("UnsignedInteger")
		)
	);
	
	DEFINE("Real",
		GLUE(
			RANGE("+-"),
			REPEAT(1, RANGE('0', '9')),
			REPEAT(0, 1,
				GLUE(
					CHAR('.'),
					REPEAT(1, RANGE('0', '9'))
				)
			),
			REPEAT(0, 1,
				GLUE(
					RANGE("eE"),
					REPEAT(0, 1,
						RANGE("+-")
					),
					REPEAT(1, RANGE('0', '9'))
				)
			)
		)
	);
	
	DEFINE("Keyword",
		KEYWORD(
			/* Turbo Pascal */ "\
			absolute array asm begin case const constructor      \
			destructor do downto else end file for function      \
			goto if implementation inherited inline interface    \
			label nil object of on operator packed               \
			procedure program record reintroduce repeat self set \
			string then to type unit until uses var while        \
			with                                                 "
			+ /* Free Pascal */ "\
			dispose exit false new true"
			+ /* Object Pascal */ "\
			class private protected public                     \
			dispinterface except exports finalization          \
			finally initialization inline library on out       \
			packed property raise resourcestring threadvar try "
		)
	);
	
	DEFINE("Operator",
		KEYWORD("\
			:= = <> < > <= >=     \
			+ - * / div mod **    \
			and not or xor        \
			<< >> shl shr         \
			>< include exclude in \
			is as                 \
		")
	);
	
	DEFINE("Modifier",
		KEYWORD("\
			absolute abstract alias assembler cdecl cppdecl  \
			default export external far far16 forward index  \
			local name near nostackframe oldfpccall override \
			pascal published read   \
			register reintroduce safecall softfloat stdcall  \
			virtual write"
		)
	);
	
	DEFINE("AddressOfIdentifier",
		GLUE(
			CHAR('@'),
			REF("Identifier")
		)
	);
	
	DEFINE_VOID("Name",
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
	
	DEFINE("Identifier",
		GLUE(
			INLINE("Name"),
			REPEAT(
				GLUE(
					CHAR('.'),
					INLINE("Name")
				)
			)
		)
	);
	
	DEFINE("Done",
		GLUE(
			PREVIOUS("Keyword", "end"),
			CHAR('.')
		)
	);
	
	DEFINE("PascalSource",
		REPEAT(
			FIND(
				CHOICE(
					REPEAT(1, RANGE(" \t\n")),
					REF("Preprocessing"),
					REF("Comment"),
					GLUE(
						REF("Keyword"),
						NOT(INLINE("Identifier"))
					),
					REF("Identifier"),
					REF("AddressOfIdentifier"),
					REF("Operator"),
					REF("String"),
					REF("Real"),
					REF("Integer"),
					REF("Done")
				)
			)
		)
	);
	
	ENTRY("PascalSource");
}
