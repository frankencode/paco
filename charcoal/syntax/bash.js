charcoal.syntax["bash"] = function()
{
	STATE_STRING("endOfDocument", "");
	
	DEFINE_VOID("WordTermination",
		AHEAD(
			CHOICE(
				INLINE("Operator"),
				RANGE(" $\t\n")
			)
		)
	);
	
	DEFINE("Operator", KEYWORD("|| & && ; ;; ( ) | ="));
	
	DEFINE("Reserved",
		GLUE(
			KEYWORD(
				"! case do done elif else esac fi for   \
				 function if in select then until while \
				 { } time [[ ]]                         "
			),
			INLINE("WordTermination")
		)
	);
	
	DEFINE("Builtin",
		GLUE(
			KEYWORD(
				"source alias bg bind break builtin cd caller      \
				 command compgen complete continue declare typeset \
				 dirs disown echo enable eval exec exit export     \
				 fc fg getopts hash help history jobs kill         \
				 let local logout popd printf pushd pwd read       \
				 readonly return set shift shopt suspend test      \
				 times trap type ulimit umask unalias unset wait   "
			),
			INLINE("WordTermination")
		)
	);
	
	DEFINE("Word",
		REPEAT(1,
			GLUE(
				NOT(INLINE("WordTermination")),
				ANY()
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
					REPEAT(RANGE('0', '7'))
				),
				GLUE(
					STRING("0x"),
					REPEAT(1,
						CHOICE(
							RANGE('0', '9'),
							RANGE('A', 'F'),
							RANGE('a', 'f')
						)
					)
				)
			),
			CHOICE(
				INLINE("WordTermination"),
				EOI()
			)
		)
	);
	
	DEFINE("EscapedChar",
		GLUE(
			CHAR('\\'), OTHER('\n')
		)
	);
	
	DEFINE("Expansion",
		CHOICE(
			GLUE(
				CHAR('$'),
				CHOICE(
					RANGE("*@#?-$!_0"),
					REPEAT(1,
						CHOICE(
							RANGE('A', 'Z'),
							RANGE('a', 'z'),
							RANGE('0', '9'),
							CHAR('_')
						)
					),
					GLUE(
						CHAR('('),
						INVOKE("bash"),
						CHAR(')')
					)
				)
			),
			GLUE(
				CHAR('`'),
				INVOKE("bash", FIND(CHAR('`')))
			)
		)
	);
	
	DEFINE("FunctionDeclarationName",
		REPEAT(1,
			CHOICE(
				RANGE('a', 'z'),
				RANGE('A', 'Z'),
				RANGE('0', '9'),
				RANGE("-_")
			)
		)
	);
	
	DEFINE("FunctionDeclaration",
		GLUE(
			// PREVIOUS("Reserved", "function"),
			// REPEAT(RANGE(" \t\r\n")),
			REF("FunctionDeclarationName"),
			REPEAT(RANGE(" \t\r\n")),
			CHAR('('),
			REPEAT(RANGE(" \t")),
			CHAR(')'),
			AHEAD(
				GLUE(
					REPEAT(RANGE(" \t\r\n")),
					CHAR('{')
				)
			)
		)
	);
	
	DEFINE("SingleQuoted",
		GLUE(
			CHAR('\''),
			FIND(CHAR('\''))
		)
	);
	
	DEFINE("DoubleQuoted",
		GLUE(
			CHAR('"'),
			REPEAT(
				CHOICE(
					REF("EscapedChar"),
					REF("Expansion"),
					OTHER('\"')
				)
			),
			CHAR('"')
		)
	);
	
	DEFINE("Document",
		GLUE(
			STRING("<<"),
			REPEAT(0, 1, CHAR('-')),
			GETSTRING("endOfDocument", FIND(AHEAD(CHAR('\n')))),
			REPEAT(
				GLUE(
					NOT(
						GLUE(
							CHAR('\n'),
							VARSTRING("endOfDocument")
						)
					),
					CHOICE(
						REF("Expansion"),
						ANY()
					)
				)
			),
			CHAR('\n'),
			VARSTRING("endOfDocument")
		)
	);
	
	DEFINE("Comment",
		GLUE(
			CHAR('#'),
			FIND(CHAR('\n'))
		)
	);
	
	DEFINE("Switch",
		GLUE(
			PREVIOUS("Reserved", "case"),
			REPEAT(
				GLUE(
					NOT(STRING("esac")),
					CHOICE(
						REF("BashEntity"),
						ANY()
					)
				)
			),
			REF("Reserved")
		)
	);
	
	DEFINE_VOID("BashEntity",
		CHOICE(
			REPEAT(1, RANGE(" \t\r\n")),
			REF("SingleQuoted"),
			REF("DoubleQuoted"),
			REF("Document"),
			REF("Comment"),
			REF("Operator"),
			GLUE(
				REF("Reserved"),
				REPEAT(0, 1, REF("Switch"))
			),
			REF("FunctionDeclaration"),
			REF("Builtin"),
			REF("Expansion"),
			REF("Integer"),
			REF("Word"),
			GLUE(
				CHAR('('),
				INVOKE("bash", FIND(CHAR(')')))
			)
		)
	);

	DEFINE("BashSource",
		REPEAT(
			GLUE(
				NOT(CHAR(')')),
				FIND(REF("BashEntity"))
			)
		)
	);
	
	ENTRY("BashSource");
}
