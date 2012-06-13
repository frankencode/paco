charcoal.syntax["cucumber"] = function()
{
	DEFINE("Comment",
		GLUE(
			CHAR('#'),
			FIND(
				CHOICE(CHAR('\n'), EOI())
			)
		)
	);

	DEFINE("Tag",
		GLUE(
			CHAR('@'),
			REPEAT(1, EXCEPT(" \t\n"))
		)
	);

	DEFINE_VOID("CommentOrWhitespace",
		CHOICE(
			RANGE(" \t\n"),
			REF("Comment"),
			REF("Tag")
		)
	);
	
	DEFINE("Entity",
		CHOICE(
			STRING("Feature"),
			GLUE(
				STRING("Scenario"),
				REPEAT(
					GLUE(
						REPEAT(1, RANGE(" \t")),
						STRING("Outline")
					)
				)
			),
			STRING("Background"),
			GLUE(STRING("Example"), REPEAT(0, 1, CHAR('s'))),
			STRING("Scenarios")
		)
	);
	
	DEFINE("FeatureKey",
		CHOICE(
			GLUE(
				STRING("In"),
				REPEAT(1, RANGE(" \t")),
				STRING("order")
			),
			GLUE(
				STRING("As"),
				REPEAT(1, RANGE(" \t")),
				CHAR("a"),
				REPEAT(0, 1, CHAR("n"))
			),
			GLUE(
				STRING("I"),
				REPEAT(1, RANGE(" \t")),
				STRING("want")
			)
		)
	);
	
	DEFINE("ScenarioKey",
		KEYWORD("Given When Then And But")
	);
	
	DEFINE_VOID("Title",
		GLUE(
			REPEAT(1, EXCEPT("\n#")),
			REPEAT(0, 1, REF("Comment"))
		)
	);
	
	STATE_CHAR("quote", '"');
	
	DEFINE("Citation",
		GLUE(
			AHEAD(RANGE("\"'`")),
			GETCHAR("quote"),
			REPEAT(
				GLUE(
					NOT(VARCHAR("quote")),
					EXCEPT('\n')
				)
			),
			VARCHAR("quote")
		)
	);
	
	DEFINE_VOID("Description",
		GLUE(
			REPEAT(1,
				CHOICE(
					REF("Citation"),
					EXCEPT("\n#")
				)
			),
			REPEAT(0, 1, REF("Comment"))
		)
	);
	
	DEFINE("FeatureTitle", INLINE("Title"));
	DEFINE("ScenarioTitle", INLINE("Title"));
	
	DEFINE("FeatureClause",
		REPEAT(1,
			GLUE(
				REPEAT(REF("CommentOrWhitespace")),
				NOT(INLINE("FeatureKey")),
				NOT(INLINE("Entity")),
				INLINE("Description")
			)
		)
	);
	
	DEFINE("ScenarioClause",
		REPEAT(1,
			GLUE(
				REPEAT(REF("CommentOrWhitespace")),
				NOT(INLINE("ScenarioKey")),
				NOT(INLINE("Entity")),
				INLINE("Description")
			)
		)
	);
	
	DEFINE("Scenario",
		GLUE(
			AHEAD(
				CHOICE(
					STRING("Scenario"),
					STRING("Background"),
					STRING("Example")
				)
			),
			REF("Entity"),
			REPEAT(RANGE(" \t")),
			CHAR(":"),
			REPEAT(RANGE(" \t")),
			REPEAT(0, 1, REF("ScenarioTitle")),
			REPEAT(
				CHOICE(
					REPEAT(1, REF("CommentOrWhitespace")),
					GLUE(
						REPEAT(0, 1,
							GLUE(
								REF("ScenarioKey"),
								REPEAT(1, RANGE(" \t"))
							)
						),
						REF("ScenarioClause")
					)
				)
			)
		)
	);
	
	DEFINE("Feature",
		GLUE(
			AHEAD(STRING("Feature")),
			REF("Entity"),
			REPEAT(RANGE(" \t")),
			CHAR(":"),
			REPEAT(RANGE(" \t")),
			REF("FeatureTitle"),
			REPEAT(
				CHOICE(
					REPEAT(1, REF("CommentOrWhitespace")),
					GLUE(
						REF("FeatureKey"),
						REPEAT(1, RANGE(" \t")),
						REF("FeatureClause")
					)
				)
			),
			REPEAT(
				FIND(
					CHOICE(
						REPEAT(1, REF("CommentOrWhitespace")),
						REF("Scenario")
					)
				)
			)
		)
	);

	DEFINE("CucumberSource",
		GLUE(
			REPEAT(REF("CommentOrWhitespace")),
			FIND(REF("Feature"))
		)
	);
	
	ENTRY("CucumberSource");
}
