charcoal.syntax["cmake"] = function()
{
	OPTION("CaseSensitive", false);
	
	STATE_CHAR("quotationMark", '\0');
	
	DEFINE("Comment",
		GLUE(
			CHAR('#'),
			FIND(CHAR('\n'))
		)
	);
	
	DEFINE("Identifier",
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
	
	DEFINE("Command",
		KEYWORD("\
			add_custom_command \
			add_custom_target \
			add_definitions \
			add_dependencies \
			add_executable \
			add_library \
			add_subdirectory \
			add_test \
			aux_source_directory \
			break \
			build_command \
			cmake_minimum_required \
			cmake_policy \
			configure_file \
			create_test_sourcelist \
			define_property \
			else \
			elseif \
			enable_language \
			enable_testing \
			endforeach \
			endfunction \
			endif \
			endmacro \
			endwhile \
			execute_process \
			export \
			file \
			find_file \
			find_library \
			find_package \
			find_path \
			find_program \
			fltk_wrap_ui \
			foreach \
			function \
			get_cmake_property \
			get_directory_property \
			get_filename_component \
			get_property \
			get_source_file_property \
			get_target_property \
			get_test_property \
			if \
			include \
			include_directories \
			include_external_msproject \
			include_regular_expression \
			install \
			link_directories \
			list \
			load_cache \
			load_command \
			macro \
			mark_as_advanced \
			math \
			message \
			option \
			output_required_files \
			project \
			qt_wrap_cpp \
			qt_wrap_ui \
			remove_definitions \
			return \
			separate_arguments \
			set \
			set_directory_properties \
			set_property \
			set_source_files_properties \
			set_target_properties \
			set_tests_properties \
			site_name \
			source_group \
			string \
			target_link_libraries \
			try_compile \
			try_run \
			unset \
			variable_watch \
			while"
		)
	);
	
	DEFINE("String",
		GLUE(
			AHEAD(RANGE("\"'")),
			GETCHAR("quotationMark"),
			FIND(VARCHAR("quotationMark"))
		)
	);
	
	DEFINE("Boolean",
		KEYWORD(
			"ON   YES  TRUE   Y        \
			 OFF  NO   FALSE  N  IGNORE"
		)
	);
	
	DEFINE("Number",
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
	
	DEFINE("Version",
		GLUE(
			REPEAT(1,
				GLUE(
					REF("Number"),
					CHAR('.')
				)
			),
			REF("Number")
		)
	);
	
	DEFINE("Function",
		GLUE(
			CHOICE(
				REF("Command"),
				REF("Identifier")
			),
			REPEAT(RANGE(" \t\n\r\f")),
			AHEAD(CHAR('('))
		)
	);
	
	DEFINE("Expansion",
		GLUE(
			REPEAT(1, 2, CHAR('$')),
			CHAR('{'),
			REF("Identifier"),
			CHAR('}')
		)
	);
	
	DEFINE("CMakeSource",
		REPEAT(
			FIND(
				CHOICE(
					REPEAT(1, RANGE(" \t\n\r\f")),
					REF("Comment"),
					REF("Function"),
					REF("String"),
					REF("Expansion"),
					REF("Identifier"),
					REF("Boolean"),
					REF("Version"),
					REF("Number")
				)
			)
		)
	);
	
	ENTRY("CMakeSource");
}
