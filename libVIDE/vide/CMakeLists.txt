SET(VIDE_SRCS
	QxControl.cpp
	QxVisual.cpp
	QxStyle.cpp
	QxTexture.cpp
	QxStyleManager.cpp
)

SET(VIDE_MOC_HDRS
	QxControl.hpp
	QxVisual.hpp
	QxStyle.hpp
	QxTexture.hpp
	QxStyleManager.hpp
)

QT4_WRAP_CPP(VIDE_MOC_SRCS ${VIDE_MOC_HDRS})

ADD_LIBRARY(vide
	${VIDE_SRCS}
	${VIDE_MOC_SRCS}
)

SET_TARGET_PROPERTIES(vide PROPERTIES VERSION 0.1.0 SOVERSION 0)

INSTALL(TARGETS vide ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
