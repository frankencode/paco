charcoal.stacks["cxx"] =
{
	title: "C/C++",
	syntax: "cxx",
	filename:
		[ "*.hpp", "*.cpp",
		  "*.hxx", "*.cxx",
		  "*.h++", "*.c++",
		  "*.hh", "*.cc",
		  "*.h", "*.c",
		  /*"*.m",*/ "*.mm", // workaround HACK, fixes collision with Octave
		  "*.y", "*.yy",
		  "*.qdoc",
		  "*.cstr" ]
};
