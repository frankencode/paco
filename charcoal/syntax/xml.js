charcoal.syntax["xml"] = function()
{
	STATE_FLAG("tagListIsOpen", false);
	STATE_CHAR("quotation", '\0');
	
	DEFINE_VOID("Whitespace", REPEAT(1, RANGE(" \t\r\n")));

	DEFINE("Comment",
		GLUE(
			STRING("<!--"),
			REPEAT(
				GLUE(
					NOT(STRING("-->")),
					ANY()
				)
			),
			STRING("-->")
		)
	);
	
	DEFINE("Tag",
		GLUE(
			REF("TagOpen"),
			REF("TagName"),
			REF("TagBody"),
			REF("TagClose")
		)
	);
	
	
	DEFINE("TagOpen",
		GLUE(
			CHAR('<'),
			REPEAT(0, 1, RANGE("/?!"))
		)
	);
		
	DEFINE("TagClose",
		GLUE(
			REPEAT(0, 1, RANGE("?/")),
			CHAR('>')
		)
	);
	
	DEFINE("TagName",
		REPEAT(1,
			GLUE(
				NOT(
					CHOICE(
						INLINE("Whitespace"),
						RANGE("/?<>([%")
					)
				),
				ANY()
			)
		)
	);
	
	DEFINE_VOID("TagBody",
		REPEAT(
			IF("tagListIsOpen",
				GLUE(
					CHAR(']'),
					SET("tagListIsOpen", false)
				),
				CHOICE(
					GLUE(
						CHAR('['),
						SET("tagListIsOpen", true)
					),
					GLUE(
						NOT(
							GLUE(
								REPEAT(0, 1, RANGE("?/")),
								CHAR('>')
							)
						),
						CHOICE(
							REF("Whitespace"),
							REF("AttributeValue"),
							REF("AttributeName"),
							ANY()
						)
					)
				)
			)
		)
	);
	
	DEFINE("AttributeName",
		GLUE(
			REPEAT(1,
				GLUE(
					NOT(
						CHOICE(
							REF("Whitespace"),
							RANGE("=['\"")
						)
					),
					ANY()
				)
			),
			AHEAD(
				GLUE(
					REPEAT(0, 1, REF("Whitespace")),
					CHAR('=')
				)
			)
		)
	);
	
	DEFINE("AttributeValue",
		GLUE(
			AHEAD(RANGE("'\"")),
			GETCHAR("quotation"),
			REPEAT(
				LENGTH(1,
					CHOICE(
						GLUE(CHAR('\\'), ANY()),
						VAROTHER("quotation")
					)
				)
			),
			VARCHAR("quotation")
		)
	);
	
	DEFINE("ScriptTagName",
		CHOICE(
			STRING("script"),
			STRING("SCRIPT")
		)
	);
	
	DEFINE("ScriptTag",
		GLUE(
			REF("TagOpen"),
			REF("ScriptTagName"),
			REF("TagBody"),
			REF("TagClose")
		)
	);
	
	DEFINE("StyleTagName",
		CHOICE(
			STRING("style"),
			STRING("STYLE")
		)
	);
	
	DEFINE("StyleTag",
		GLUE(
			REF("TagOpen"),
			REF("StyleTagName"),
			REF("TagBody"),
			REF("TagClose")
		)
	);
	
	DEFINE("Script",
		GLUE(
			REF("ScriptTag"),
			INVOKE("javascript",
				FIND(
					AHEAD(
						INLINE("ScriptTag")
					)
				)
			),
			REF("ScriptTag")
		)
	);
	
	DEFINE("Style",
		GLUE(
			REF("StyleTag"),
			INVOKE("css",
				FIND(
					AHEAD(
						INLINE("StyleTag")
					)
				)
			),
			REF("StyleTag")
		)
	);
	
	DEFINE("XmlSource",
		REPEAT(
			FIND(
				CHOICE(
					INLINE("Whitespace"),
					REF("Comment"),
					REF("Script"),
					REF("Style"),
					REF("Tag")
				)
			)
		)
	);
	
	ENTRY("XmlSource");
}
