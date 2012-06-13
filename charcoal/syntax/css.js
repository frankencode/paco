charcoal.syntax["css"] = function()
{
	OPTION("CaseSensitive", false);
	
	STATE_CHAR("quotationMark", '\0');
	
	DEFINE("Comment",
		GLUE(
			STRING("/*"),
			FIND(STRING("*/"))
		)
	);
	
	DEFINE("Integer",
		GLUE(
			REPEAT(0, 1, RANGE("+-")),
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
		)
	);
	
	DEFINE("Float",
		GLUE(
			REPEAT(0, 1, RANGE("+-")),
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
	
	DEFINE("Property",
		KEYWORD(
			"azimuth \
			 background-attachment \
			 background-color \
			 background-image \
			 background-position \
			 background-repeat \
			 background \
			 border-collapse \
			 border-color \
			 border-spacing \
			 border-style \
			 border-top \
			 border-right \
			 border-bottom \
			 border-left \
			 border-top-color \
			 border-right-color \
			 border-bottom-color \
			 border-left-color \
			 border-top-style \
			 border-right-style \
			 border-bottom-style \
			 border-left-style \
			 border-top-width \
			 border-right-width \
			 border-bottom-width \
			 border-left-width \
			 border-width \
			 border \
			 bottom \
			 caption-side \
			 clear \
			 clip \
			 color \
			 content \
			 counter-increment \
			 counter-reset \
			 cue-after \
			 cue-before \
			 cue \
			 cursor \
			 direction \
			 display \
			 elevation \
			 empty-cells \
			 float \
			 font-family \
			 font-size \
			 font-style \
			 font-variant \
			 font-weight \
			 font \
			 height \
			 left \
			 letter-spacing \
			 line-height \
			 list-style-image \
			 list-style-position \
			 list-style-type \
			 list-style \
			 margin-right \
			 margin-left \
			 margin-top \
			 margin-bottom \
			 margin \
			 max-height \
			 max-width \
			 min-height \
			 min-width \
			 orphans \
			 outline-color \
			 outline-style \
			 outline-width \
			 outline \
			 overflow \
			 padding-top \
			 padding-right \
			 padding-bottom \
			 padding-left \
			 padding \
			 page-break-after \
			 page-break-before \
			 page-break-inside \
			 pause-after \
			 pause-before \
			 pause \
			 pitch-range \
			 pitch \
			 play-during \
			 position \
			 quotes \
			 richness \
			 right \
			 speak-header \
			 speak-numeral \
			 speak-punctuation \
			 speak \
			 speech-rate \
			 stress \
			 table-layout \
			 text-align \
			 text-decoration \
			 text-indent \
			 text-transform \
			 top \
			 unicode-bidi \
			 vertical-align \
			 visibility \
			 voice-family \
			 volume \
			 white-space \
			 widows \
			 width \
			 word-spacing \
			 z-index \
			 "
		)
	);
	
	DEFINE_VOID("Name",
		GLUE(
			CHOICE(
				RANGE('a', 'z'),
				RANGE('A', 'Z'),
				RANGE("_-")
			),
			REPEAT(
				CHOICE(
					RANGE('a', 'z'),
					RANGE('A', 'Z'),
					RANGE("_-"),
					RANGE('0', '9')
				)
			)
		)
	);
	
	DEFINE("UnknownProperty",
		INLINE("Name")
	);
	
	DEFINE("String",
		CHOICE(
			INLINE("Name"),
			GLUE(
				AHEAD(RANGE("\"'")),
				GETCHAR("quotationMark"),
				FIND(VARCHAR("quotationMark"))
			)
		)
	);
	
	DEFINE("Unit",
		GLUE(
			KEYWORD(
				"em ex px cm mm in pt pc deg rad grad ms s hz khz"
			),
			NOT(
				INLINE("String")
			)
		)
	);
	
	DEFINE("Value",
		CHOICE(
			GLUE(
				REPEAT(1,
					GLUE(
						CHOICE(
							REF("Number"),
							REF("String")
						),
						REPEAT(0, RANGE(" \t")),
						REPEAT(0, 1, REF("Unit")),
						REPEAT(0, RANGE(" \t")),
						REPEAT(0, 1, CHAR(',')),
						REPEAT(0, RANGE(" \t"))
					)
				),
				AHEAD(
					RANGE("\n;}")
				)
			),
			FIND(
				AHEAD(
					RANGE("\n;}")
				)
			)
		)
	);
	
	DEFINE("Block",
		GLUE(
			CHAR('{'),
			REPEAT(
				GLUE(
					NOT(CHAR('}')),
					CHOICE(
						REPEAT(1, RANGE(" \t\n\r\f")),
						REF("Comment"),
						GLUE(
							CHOICE(
								REF("Property"),
								REF("UnknownProperty")
							),
							REPEAT(0, 1,
								GLUE(
									REPEAT(0, RANGE(" \t")),
									CHAR(":"),
									REPEAT(0, RANGE(" \t")),
									REF("Value"),
									REPEAT(0, RANGE(" \t")),
									REPEAT(0, 1, CHAR(';'))
								)
							)
						),
						ANY()
					)
				)
			),
			CHAR('}')
		)
	);
	
	DEFINE("Rule",
		GLUE(
			CHAR('@'),
			INLINE("Name")
		)
	);
	
	DEFINE("Tag",
		INLINE("Name")
	);
	
	DEFINE("Class",
		GLUE(
			CHAR('.'),
			INLINE("Name")
		)
	);
	
	DEFINE("Id",
		GLUE(
			CHAR('#'),
			INLINE("Name")
		)
	);
	
	DEFINE("PseudoClass",
		GLUE(
			CHAR(':'),
			INLINE("Name")
		)
	);
	
	DEFINE("Selector",
		CHOICE(
			REF("Tag"),
			REF("Class"),
			REF("Id"),
			REF("PseudoClass")
		)
	);
	
	DEFINE("CssSource",
		REPEAT(
			FIND(
				CHOICE(
					REPEAT(1, RANGE(" \t\n\r\f")),
					REF("Comment"),
					REF("Block"),
					REF("Rule"),
					REF("Selector")
				)
			)
		)
	);
	
	ENTRY("CssSource");
}
