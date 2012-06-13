charcoal.syntax["dao"] = function()
{
	DEFINE("EscapedChar",
		GLUE(
			CHAR('\\'),
			CHOICE(
				// FIXME, unicode sequences missing here?
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
					EXCEPT("\"\n")
				)
			),
			CHAR('"')
		)
	);
	
	DEFINE_VOID("IdChar",
		CHOICE(
			RANGE('a', 'z'),
			RANGE('A', 'Z'),
			RANGE('0', '9'),
			CHAR('_')
		)
	);
	
	DEFINE("Keyword",
		GLUE(
			KEYWORD(
				"use load import require by as syntax typedef final class     \
				 routine function self any                                    \
				 reflect and or not if else elif elseif for in                \
				 do while until switch case default break skip return yield   \
				 my const local global enum private protected public virtual  \
				 try retry rescue raise async hurry join extern               "
				 //  FIXME curry?-)
			),
			NOT(INLINE("IdChar"))
		)
	);
	
	DEFINE("CoreTypes",
		GLUE(
			KEYWORD(
				"? int float double complex string long\
				 array list map pair tuple stream      "
			),
			NOT(INLINE("IdChar"))
		)
	);
	
	DEFINE("Builtin",
		GLUE(
			KEYWORD(
				"coroutine thread mutex condition semaphore\
				 buffer math stdio stdlib network mtlib mpi"
			),
			NOT(INLINE("IdChar"))
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
	
	DEFINE("Integer",
		GLUE(
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
				GLUE(CHAR('0'), REPEAT(1, RANGE('0', '7'))),
				REPEAT(1, RANGE('0', '9'))
			),
			REPEAT(0, 1, RANGE("uU")),
			REPEAT(0, 2, RANGE("lL"))
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
	
	DEFINE("Char",
		GLUE(
			CHAR('\''),
			CHOICE(
				REF("EscapedChar"),
				OTHER('\'')
			),
			CHAR('\'')
		)
	);
	
	DEFINE("LineComment",
		GLUE(
			CHAR('#'),
			FIND(CHAR('\n'))
		)
	);
	
	DEFINE("BlockComment",
		GLUE(
			// FIXME, block comments can be cascaded?
			STRING("#{"),
			FIND(STRING("#}"))
		)
	);
	
	DEFINE("DaoSource",
		REPEAT(
			FIND(
				CHOICE(
					REF("BlockComment"),
					REF("LineComment"),
					REF("String"),
					REF("Keyword"),
					REF("CoreTypes"),
					REF("Builtin"),
					REF("Name"),
					REF("Float"),
					REF("Integer"),
					REF("Char")
				)
			)
		)
	);
	
	ENTRY("DaoSource");
}
