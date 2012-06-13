SET(PTE_SRCS
	Document.cpp
	ScriptableSyntaxDefinition.cpp
	DocumentColorizer.cpp
	SequenceAdapter.cpp
	Highlighter.cpp
	View.cpp
	ViewMetrics.cpp
	Edit.cpp
	LanguageLayer.cpp
	LanguageStack.cpp
	LanguageManager.cpp
	HighlightingTheme.cpp
	SyntaxStyle.cpp
	ThemeManager.cpp
	Palette.cpp
	PaletteManager.cpp
	CharcoalDirectory.cpp
	InsightParser.cpp
	InsightModel.cpp
	InsightTree.cpp
	EditWidget.cpp
	Context.cpp
	ScrollMachine.cpp
	SearchPopup.cpp
	MembersTipModel.cpp
	QuickHelp.cpp
	TextElider.cpp
	SearchPatternHighlighter.cpp
)

SET(PTE_MOC_HDRS
	Document.hpp
	Highlighter.hpp
	InsightModel.hpp
	ScriptableSyntaxDefinition.hpp
	ThemeManager.hpp
	View.hpp
	Edit.hpp
	EditWidget.hpp
	Context.hpp
	ScrollMachine.hpp
	SearchPopup.hpp
	MembersTipModel.hpp
	QuickHelp.hpp
	TextElider.hpp
	SearchPatternHighlighter.hpp
)

QT4_WRAP_CPP(PTE_MOC_SRCS ${PTE_MOC_HDRS})

ADD_LIBRARY(pte
	${PTE_SRCS}
	${PTE_MOC_SRCS}
)

INCLUDE_DIRECTORIES(
	${PROJECT_BINARY_DIR} # for including ui headers
	${QT_QTSCRIPT_INCLUDE_DIR}
)

SET_TARGET_PROPERTIES(pte PROPERTIES VERSION 0.1.0 SOVERSION 0)

INSTALL(TARGETS pte ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
