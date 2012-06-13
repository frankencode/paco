charcoal.syntax["sql"] = function()
{
	OPTION("CaseSensitive", false);
	
	STATE_FLAG("escaped", false);
	STATE_CHAR("quotation", '\0');
	
	DEFINE("LineComment",
		GLUE(
			STRING("--"),
			FIND(AHEAD(CHAR('\n')))
		)
	);
	
	DEFINE("BlockComment",
		GLUE(
			STRING("/*"),
			REPEAT(
				CHOICE(
					INLINE("BlockComment"),
					INLINE("LineComment"),
					GLUE(
						NOT(STRING("*/")),
						ANY()
					)
				)
			),
			FIND(STRING("*/"))
		)
	);
	
	DEFINE("Comment",
		CHOICE(
			REF("LineComment"),
			REF("BlockComment")
		)
	);
	
	DEFINE("Keyword",
		GLUE(
			KEYWORD("\
				explain query plan \
				alter table rename to add column \
				analyze \
				attach database as \
				begin deferred immediate exclusive transaction \
				commit end \
				rollback savepoint \
				release \
				create unique index if not exists \
				collate asc desc \
				temp temporary \
				primary foreign key references \
				autoincrement null check default \
				constraint \
				on delete update set cascade restrict action \
				deferrable initially immediate \
				conflict abort fail ignore replace \
				trigger before after instead of \
				for each row when \
				virtual using \
				delete from where \
				ordery by limit offset \
				detach database \
				drop view \
				distinct cast like glob escape regexp match isnull notnull \
				between and in else \
				raise current_time current_date current_timestamp \
				values pragma reindex group having \
				natural left inner cross outer join \
				union intersect except all \
				indexed vaccuum "
				// keywords from Sqlite documentation
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
	
	DEFINE("Datatype",
		GLUE(
			KEYWORD("\
				null integer real text blob \
				int tinyint smallint mediumint bigint \
				unsigned character varchar nchar \
				double precision float numeric decimal boolean \
				data datetime \
				"
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
	
	DEFINE("EscapeSequence",
		GLUE(
			CHAR('\\'),
			CHOICE(
				RANGE("bfnrt\\'"),
				REPEAT(1, 3,
					RANGE('0', '7')
				),
				GLUE(
					CHAR('x'),
					REPEAT(1, 2,
						CHOICE(
							RANGE('0', '9'),
							RANGE('A', 'F'),
							RANGE('a', 'f')
						)
					)
				)
			)
		)
	);
	
	DEFINE("String",
		GLUE(
			CHOICE(
				GLUE(
					CHAR('E'),
					SET("escaped", true)
				),
				SET("escaped", false)
			),
			CHAR('\''),
			REPEAT(
				CHOICE(
					IF("escaped",
						REF("EscapeSequence"),
						FAIL()
					),
					STRING("''"),
					OTHER('\'')
				)
			),
			CHAR('\'')
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
		CHOICE(
			REF("Float"),
			REF("Integer")
		)
	);
	
	DEFINE("Name",
		GLUE(
			CHOICE(
				RANGE('A', 'Z'),
				RANGE('a', 'z'),
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
		CHOICE(
			REF("Name"),
			GLUE(
				CHOICE(
					GLUE(
						AHEAD(RANGE("\"`")),
						GETCHAR("quotation")
					),
					GLUE(
						CHAR('['),
						SETCHAR("quotation", ']')
					)
				),
				REF("Name"),
				VARCHAR("quotation")
			)
		)
	);
	
	DEFINE("Operator",
		RANGE("+ - * / < > = ~ ! @ # % ^ & | ` ?")
	);
	
	DEFINE("SqlSource",
		REPEAT(
			FIND(
				CHOICE(
					REPEAT(1, RANGE(" \t\n\r\f")),
					REF("Comment"),
					REF("String"),
					REF("Keyword"),
					REF("Datatype"),
					REF("Identifier"),
					REF("Number"),
					REF("Operator")
				)
			)
		)
	);
	
	ENTRY("SqlSource");
}
