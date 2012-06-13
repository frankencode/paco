#include "EscapeSequenceSyntax.hpp"

namespace pacu
{

EscapeSequenceSyntax::EscapeSequenceSyntax()
{
	number_ = DEFINE("number", REPEAT(1, 4, RANGE('0', '9')));
	title_ = DEFINE("title", REPEAT(1, RANGE(32, 255)));
	
	controlSequence_ =
		DEFINE("controlSequence",
			GLUE(
				CHAR(27),
				CHOICE(
					GLUE(
						// csi sequences
						CHAR('['),
						REPEAT(0, 1, RANGE("?>")),
						REPEAT(0, 15,
							GLUE(
								REF("number"),
								REPEAT(0, 1, CHAR(';'))
							)
						),
						RANGE(32, 255)
					),
					GLUE(
						// osc sequences
						CHAR(']'),
						REF("number"),
						CHAR(';'),
						REF("title"),
						CHAR(7)
					),
					GLUE(
						// two or three byte escape sequences
						REPEAT(0, 1,
							// character set selection, etc.
							RANGE("()%")
						),
						// other
						RANGE(33, 127)
					)
				)
			)
		);
	
	ENTRY("controlSequence");
	LINK();
}

} // namespace pacu
