INCLUDE_DIRECTORIES(
	..
	${PROJECT_BINARY_DIR} # for including ui headers
	${QT_QTSCRIPT_INCLUDE_DIR}
)

LINK_LIBRARIES(
	pte
	${QT_LIBRARIES}
	${QT_QTSCRIPT_LIBRARIES}
)

ADD_EXECUTABLE(testSequenceAdapter testSequenceAdapter.cpp)

#SET(testDocumentSyntax_SRCS )
#SET(testDocumentSyntax_MOC_HDRS )
#QT4_WRAP_CPP(testDocumentSyntax_MOC_SRCS ${testDocumentSyntax_MOC_HDRS})

ADD_EXECUTABLE(testDocumentSyntax
	testDocumentSyntax.cpp
#	${testDocumentSyntax_SRCS}
#	${testDocumentSyntax_MOC_SRCS}
)
