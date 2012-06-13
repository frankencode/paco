charcoal.syntax["octave"] = function()
{
	STATE_CHAR("CommentMark", 0);
	STATE_CHAR("StringDelimiter", 0);
	
	// FIXME: strings can't be cascaded
	DEFINE("Comment",
		GLUE(
			AHEAD(RANGE("#%")),
			GETCHAR("CommentMark"),
			CHOICE(
				GLUE(
					CHAR('{'),
					FIND(
						GLUE(
							VARCHAR("CommentMark"),
							CHAR('}')
						)
					)
				),
				FIND(
					CHOICE(
						CHAR('\n'),
						EOI()
					)
				)
			)
		)
	);
	
	DEFINE("BuiltinAtomic",
		KEYWORD("int8 uint8 int16 uint16 int32 uint32 int64 uint64 single double")
	);
	
	DEFINE("Builtin",
		KEYWORD("\
			class isa cast typecast swapbytes \
			realmin realmax eps \
			NA isna \
			isnan \
			toascii ischar char \
			who whos who_line_format exist \
			options which what clear \
			disp input yes_or_no kbhit \
			rats \
			eval feval run \
			ans isvarname genvarname namelengthmax \
			isequal isequalwithequalnans \
			nargin nargout inputname  \
			mfilename  source \
			functions func2str str2func \
			inline argnames formula vectorize \
			error usage \
			isobject methods ismethod \
			plus minus uplus uminus time mtimes rdivide mrdivide ldivide mldivide \
			lt le gt ge eq ne \
			and or not ctranspose transpose colon horzcat vertcat \
			subsref subsasgn subsindex display \
			\
			ismatrix isvector iscell isscalar iscomplex \
			length floor true false min max abs find warning error \
		")
/* FIXME: this language lacks OO design,...
			silent_function
			beep_on_error
			ignore_function_time_stamp
			default_save_options save_precision save_header_format_string native_float_format fdisp \
			mark_as_command unmark_command iscommand mark_as_rawcommand unmark_raw_command israwcommand \
			debug_on_interrupt debug_on_warning debug_on_error keyboard \
			
			stdin stdout sterr fopen fclose fseek ftell ferror fgets fputs fread fwrite \
			fgetl \
			fscanf scanf sscanf \
			fprintf printf sprintf \
			mkstemp tmpfile tmpnam \
			rename link symlink readlink unlink readdir mkdir rmmdir confirm_recursive_rmdir mkfifo \
			umask stat lstat fstat glob fnmatch file_is_path canonicalize_file_name \
			filesep filemarker P_tmpdir \
			is_absolute_filename is_rooted_relative_filename make_absolute_filename \
*/
	);
	
	DEFINE("EscapeSequence",
		GLUE(
			CHAR('\\'),
			RANGE("\\\"'0abfnrtv")
		)
	);
	
	DEFINE("DoubleQuotedString",
		GLUE(
			CHAR('"'),
			REPEAT(
				CHOICE(
					REF("EscapeSequence"),
					OTHER('"') // FIXME, should say: EXCEPT("\"\n") ?
				)
			),
			CHAR('"')
		)
	);
	
	DEFINE("EscapedSingleQuote",
		STRING("''")
	);
	
	DEFINE("SingleQuotedString",
		GLUE(
			CHAR('\''),
			REPEAT(
				CHOICE(
					REF("EscapedSingleQuote"),
					OTHER('\'') // FIXME, should say: EXCEPT("'\n")
				)
			),
			CHAR('\'')
		)
	);
	
	DEFINE("Punctuator",
		RANGE("{}[];,:.")
	);
	
	DEFINE("Continuation",
		CHOICE(
			STRING("..."),
			CHAR('\\')
		)
	);
	
	DEFINE("ReservedWord",
		KEYWORD("\
			if elseif else endif \
			switch case otherwise endswitch \
			while endwhile \
			do until \
			for endfor \
			break continue \
			unwind_protect unwind_protect_cleanup end_unwind_protect \
			try catch end_try_catch \
			global isglobal \
			persistent \
			function endfunction return \
		")
	);
	
	DEFINE("Operator", // NOTE: needs to be matched after SingleQuotedString
		KEYWORD("\
			+ .+ - .- * .* / ./ \\ .\\ ^ ** .^ .** ' .' \
			< <= == >= > != ~= \
			& | ! ~ \
			&& || \
			= \
			++ -- \
		")
	);
	
	DEFINE("InternalIdentifier",
		GLUE(
			STRING("__"),
			INLINE("Identifier")
		)
	);
	
	DEFINE("Identifier",
		GLUE(
			CHOICE(
				RANGE('a', 'z'),
				CHAR('_'),
				RANGE('A', 'Z')
			),
			REPEAT(
				CHOICE(
					RANGE('a', 'z'),
					CHAR('_'),
					RANGE('0', '9'),
					RANGE('A', 'Z')
				)
			)
		)
	);
	
	DEFINE("FunctionHandle",
		GLUE(
			CHAR('@'),
			REF("Identifier")
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
	
	DEFINE("OctaveSource",
		REPEAT(
			FIND(
				CHOICE(
					REPEAT(1, RANGE(" \t\n")),
					REF("Comment"),
					GLUE(
						CHOICE(
							REF("Builtin"),
							REF("BuiltinAtomic")
						),
						NOT(INLINE("Identifier"))
					),
					GLUE(
						REF("ReservedWord"),
						NOT(INLINE("Identifier"))
					),
					REF("InternalIdentifier"),
					REF("Identifier"),
					REF("Float"),
					REF("Integer"),
					REF("Operator"),
					REF("SingleQuotedString"),
					REF("DoubleQuotedString"),
					REF("Punctuator"),
					REF("FunctionHandle"),
					REF("Continuation")
				)
			)
		)
	);
	
	ENTRY("OctaveSource");
}
