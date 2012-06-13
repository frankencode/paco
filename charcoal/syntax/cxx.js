charcoal.syntax["cxx"] = function()
{
	DEFINE("DocCommentOpen", CHOICE(STRING("/*!"), STRING("//!")));
	DEFINE("DocCommentClose", STRING("*/"));
	
	DEFINE("DocComment",
		CHOICE(
			GLUE(
				AHEAD(STRING("/*!")),
				REF("DocCommentOpen"),
				INVOKE("qdoc", FIND(AHEAD(STRING("*/")))),
				REF("DocCommentClose")
			),
			GLUE(
				AHEAD(STRING("//!")),
				REF("DocCommentOpen"),
				INVOKE("qdoc", FIND(AHEAD(CHAR('\n'))))
			)
		)
	);
	
	DEFINE("BlockComment",
		GLUE(
			STRING("/*"),
			FIND(STRING("*/"))
		)
	);
	
	DEFINE("LineComment",
		GLUE(
			STRING("//"),
			FIND(AHEAD(CHAR('\n')))
		)
	);
	
	DEFINE_VOID("PreprocessingConditional",
		GLUE(
			REPEAT(RANGE(" \t")),
			CHAR('#'),
			REPEAT(RANGE(" \t")),
			STRING("if"),
			REPEAT(
				CHOICE(
					REF("PreprocessingConditional"),
					GLUE(
						NOT(
							GLUE(
								CHAR('\n'),
								REPEAT(RANGE(" \t")),
								CHAR('#'),
								REPEAT(RANGE(" \t")),
								STRING("endif")
							)
						),
						ANY()
					)
				)
			),
			GLUE(
				CHAR('\n'),
				REPEAT(RANGE(" \t")),
				CHAR('#'),
				REPEAT(RANGE(" \t")),
				STRING("endif")
			)
		)
	);
	
	DEFINE_VOID("PreprocessingCommentBegin",
		GLUE(
			AHEAD(
				GLUE(
					REPEAT(RANGE(" \t")),
					CHAR('#'),
					REPEAT(RANGE(" \t")),
					STRING("if"),
					REPEAT(RANGE(" \t")),
					CHAR('0'),
					REPEAT(1, RANGE(" \t\n"))
				)
			),
			REF("Preprocessing"),
			CHAR('\n')
		)
	);
	
	DEFINE_VOID("PreprocessingCommentEnd",
		GLUE(
			AHEAD(
				GLUE(
					CHAR('\n'),
					REPEAT(RANGE(" \t")),
					CHAR('#'),
					REPEAT(RANGE(" \t")),
					CHOICE(
						STRING("endif"),
						STRING("else")
					)
				)
			),
			CHAR('\n'),
			REF("Preprocessing")
		)
	);
	
	DEFINE("PreprocessingComment",
		GLUE(
			REF("PreprocessingCommentBegin"),
			REPEAT(
				CHOICE(
					REF("PreprocessingConditional"),
					GLUE(
						NOT(REF("PreprocessingCommentEnd")),
						ANY()
					)
				)
			),
			REF("PreprocessingCommentEnd")
		)
	);
	
	DEFINE("Preprocessing",
		GLUE(
			REPEAT(RANGE(" \t")),
			CHAR('#'),
			REPEAT(
				CHOICE(
					REF("BlockComment"),
					REF("LineComment"),
					STRING("\\\n"),
					OTHER('\n')
				)
			)
		)
	);
	
	DEFINE("EscapedChar",
		GLUE(
			CHAR('\\'),
			CHOICE(
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
	
	DEFINE("ObjcEscapedChar",
		INLINE("EscapedChar")
	);
	
	DEFINE("ObjcString",
		GLUE(
			CHAR('@'),
			CHAR('"'),
			REPEAT(
				CHOICE(
					REF("ObjcEscapedChar"),
					EXCEPT("\"\n")
				)
			),
			CHAR('"')
		)
	);
	
	DEFINE("Punctuator",
		KEYWORD(
			"{    }    (    )    [    ]    :   ... ; \
			 <    >    ,    *    &    =    ~         \
			 ##   ?    .    .*                       \
			 +    -    /    %    ^    |    !         \
			 +=   -=   *=   /=   %=   ^=   &=  |=    \
			 <<   >>   <<=  >>=  ==   !=   <=  >=    \
			 &&   ||   ++   --   ->   ->*            "
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
			KEYWORD("\
				namespace using \
				export template typename enum \
				class union struct typedef friend operator \
				public private protected \
				virtual explicit inline throw \
				auto register static extern mutable \
				new delete new[] delete[] \
				and and_eq asm bitand bitor \
				break case catch \
				compl const_cast continue default delete \
				do dynamic_cast else \
				false for \
				goto if \
				new not not_eq or \
				or_eq reinterpret_cast \
				return sizeof static_cast \
				switch this true \
				try typeid \
				while \
				xor xor_eq"
			),
			NOT(INLINE("IdChar"))
		)
	);
	
	DEFINE("TypeKeyword",
		GLUE(
			KEYWORD("\
				const volatile \
				void \
				signed unsigned \
				long short \
				int bool float double \
				char wchar_t \
				uint8_t uint16_t uint32_t uint64_t \
				int8_t int16_t int32_t int64_t \
				float32_t float64_t \
				size_t ssize_t off_t"
			),
			NOT(INLINE("IdChar"))
		)
	);
	
	DEFINE("QtKeyword",
		GLUE(
			CHOICE(
				GLUE(
					STRING("Q_"),
					RANGE('A', 'Z'),
					REPEAT(
						CHOICE(
							RANGE('A', 'Z'),
							RANGE('0', '9'),
							CHAR('_')
						)
					)
				),
				KEYWORD("\
					SIGNAL SLOT \
					signals slots connect disconnect \
					emit tr qobject_cast foreach \
					qscriptvalue_cast"
				)
			),
			NOT(INLINE("IdChar"))
		)
	);
	
	DEFINE("ObjcKeyword",
		GLUE(
			CHAR('@'),
			KEYWORD("\
				interface implementation protocol end \
				private protected public \
				try catch throw finally \
				property synthesize dynamic \
				class selector protocol encode synchronized"
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
	
	DEFINE("Identifier",
		REPEAT(1,
			GLUE(
				REPEAT(0, 1, STRING("::")),
				REF("Name")
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
	
	DEFINE("CxxSource",
		REPEAT(
			FIND(
				CHOICE(
					REPEAT(1, RANGE(" \t")),
					REF("DocComment"),
					REF("BlockComment"),
					REF("LineComment"),
					GLUE(
						CHOICE(
							CHAR('\n'),
							BOI()
						),
						CHOICE(
							REF("PreprocessingComment"),
							// REF("PreprocessingConditional"),
							REF("Preprocessing")
						)
					),
					CHAR('\n'),
					REF("String"),
					REF("ObjcString"),
					REF("Punctuator"),
					REF("Keyword"),
					REF("TypeKeyword"),
					REF("QtKeyword"),
					REF("ObjcKeyword"),
					REF("Identifier"),
					REF("Float"),
					REF("Integer"),
					REF("Char")
				)
			)
		)
	);
	
	ENTRY("CxxSource");
}
