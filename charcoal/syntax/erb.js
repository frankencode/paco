charcoal.syntax["erb"] = function()
{
	DEFINE("ErbComment",
		GLUE(
			STRING("<%#"),
			FIND(STRING("%>"))
		)
	);
	
	DEFINE("ErbStart",
		GLUE(
			STRING("<%"),
			REPEAT(0, 1, CHAR('='))
		)
	);
	
	DEFINE("ErbEnd",
		GLUE(
			REPEAT(0, 1, CHAR('-')),
			STRING("%>")
		)
	);
	
	DEFINE("ErbTag",
		GLUE(
			REF("ErbStart"),
			INVOKE("ruby",
				FIND(
					AHEAD(INLINE("ErbEnd"))
				)
			),
			REF("ErbEnd")
		)
	);
	
	DEFINE("EmbeddedRubySource",
		REPEAT(
			FIND(
				CHOICE(
					REF("ErbComment"),
					REF("ErbTag")
				)
			)
		)
	);
	
	ENTRY("EmbeddedRubySource");
}
