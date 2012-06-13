SET(CODETIPS_HEADER
	codetips
	codetips.hpp
	Assistant.hpp
	AssistantManager.hpp
	HelloWorld.hpp
	HaxeCodetips.hpp
	HaxeMessageSyntax.hpp
	InterpositionServer.hpp
	# CxxAutotext.hpp
	# RubyAutotext.hpp
	# PythonAutotext.hpp
	XmlAutotext.hpp
	XmlOpenTagSyntax.hpp
	WordPredictor.hpp
)

SET(CODETIPS_SOURCE
	Assistant.cpp
	AssistantManager.cpp
	HelloWorld.cpp
	HaxeCodetips.cpp
	HaxeMessageSyntax.cpp
	InterpositionServer.cpp
	# CxxAutotext.cpp
	# RubyAutotext.cpp
	# PythonAutotext.cpp
	XmlAutotext.cpp
	XmlOpenTagSyntax.cpp
	WordPredictor.cpp
)

ADD_LIBRARY(codetips
	${CODETIPS_HEADER}
	${CODETIPS_SOURCE}
)

ADD_LIBRARY(codetipsclient
	InterpositionClient.hpp
	InterpositionClient.cpp
)

SET_TARGET_PROPERTIES(codetips PROPERTIES VERSION 0.0.1 SOVERSION 0)
SET_TARGET_PROPERTIES(codetipsclient PROPERTIES VERSION 0.0.1 SOVERSION 0)

INSTALL(TARGETS codetips ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
INSTALL(TARGETS codetipsclient ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
