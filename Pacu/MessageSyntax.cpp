#include "MessageSyntax.hpp"

namespace pacu
{

MessageSyntax::MessageSyntax()
{
	path_ =
		DEFINE("path",
			LENGTH(5,
				GLUE(
					NOT(INLINE("number")),
					REPEAT(1, EXCEPT("\t :()\"'"))
				)
			)
		);
	
	number_ =
		DEFINE("number",
			GLUE(
				RANGE('1', '9'),
				REPEAT(RANGE('0', '9'))
			)
		);
	
	row_ = DEFINE("row", INLINE("number"));
	col_ = DEFINE("col", INLINE("number"));
	type_ = DEFINE("type", CHOICE(STRING("error"), STRING("warning")));
	pattern_ = DEFINE("pattern", REPEAT(1, OTHER('\'')));
	
	text_ =
		DEFINE("text",
			GLUE(
				AHEAD(
					CHOICE(
						RANGE('A', 'Z'),
						RANGE('a', 'z')
					)
				),
				REPEAT(0, 1,
					GLUE(
						REPEAT(0, 1, STRING("in ")),
						CHAR('\''),
						REF("pattern"),
						CHAR('\''),
						CHAR(':')
					)
				),
				FIND(
					GLUE(
						REPEAT(RANGE(" \t")),
						EOI()
					)
				)
			)
		);
	
	message_ =
		DEFINE("message",
			CHOICE(
				GLUE(
				    REPEAT(0, 1, STRING("file://")),
					REF("path"),
					CHAR(':'),
					REF("row"),
					NOT(
						CHOICE(
							RANGE("%,."),
							RANGE('A', 'Z'),
							RANGE('a', 'z')
						)
					),
					REPEAT(0, 1,
						GLUE(
							REPEAT(RANGE(" \t")),
							CHAR(':'),
							REPEAT(0, 1,
								CHOICE(
									GLUE(
										REF("col"),
										CHAR(':')
									),
									GLUE(
										// haXe message format
										REPEAT(RANGE(" \t")),
										STRING("character"),
										REPEAT(0, 1, CHAR('s')),
										REPEAT(RANGE(" \t")),
										INLINE("number"),
										CHAR('-'),
										REF("col"),
										REPEAT(RANGE(" \t")),
										CHAR(':')
									)
								)
							),
							REPEAT(RANGE(" \t")),
							REF("text")
						)
					)
				),
				// Python message format
				GLUE(
					STRING("File"),
					REPEAT(RANGE(" \t")),
					CHAR('"'),
					REF("path"),
					CHAR('"'),
					REPEAT(RANGE(" \t")),
					CHAR(','),
					REPEAT(RANGE(" \t")),
					STRING("line"),
					REPEAT(RANGE(" \t")),
					REF("row"),
					REPEAT(RANGE(" \t")),
					CHAR(','),
					REPEAT(RANGE(" \t")),
					REF("text")
				)
			)
		);
	
	ENTRY("message");
	LINK();
}

} // namespace pacu
