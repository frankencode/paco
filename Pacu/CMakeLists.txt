CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(Pacu)

FIND_PACKAGE(Codetips REQUIRED)
INCLUDE(UseCodetips)

FIND_PACKAGE(VIDE REQUIRED)
INCLUDE(UseVIDE)

FIND_PACKAGE(PTE REQUIRED)
INCLUDE(UsePTE)

FIND_PACKAGE(SEAL REQUIRED)
INCLUDE(UseSEAL)

FIND_PACKAGE(FTL REQUIRED)
INCLUDE(UseFTL)

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

SET(Pacu_SRCS
	QxCustomStyle.cpp
	QxWrapperStyle.cpp
	QxWidgetStack.cpp
	QxTabBar.cpp
	QxTabWidget.cpp
	QxComboBox.cpp
	QxFileSystemWatcher.cpp
	QxTerminalStack.cpp
	QxFileOpenRequest.cpp
	QxMainWindow.cpp
	QxMonospaceFilter.cpp
	QxThemesModel.cpp
	QxCommandsModel.cpp
	QxEditorPreferences.cpp
	QxTerminalPreferences.cpp
	QxPrintingPreferences.cpp
	QxCommandsPreferences.cpp
	QxCodetipsPreferences.cpp
	QxMiscPreferences.cpp
	QxPreferences.cpp
	QxCommand.cpp
	QxCommandEditor.cpp
	QxStatusBar.cpp
	QxDialog.cpp
	QxIconSelector.cpp
	QxApplication.cpp
	QxSplitter.cpp
	QxSplitterHandle.cpp
	QxFileSystemView.cpp
	QxUrlListModel.cpp
	QxUrlListView.cpp
	QxFileBrowser.cpp
	QxCodeBrowser.cpp
	QxFileFinder.cpp
	QxFileSearchEdit.cpp
	QxListPopup.cpp
	QxFileSearchItem.cpp
	QxMatchView.cpp
	QxFindText.cpp
#	QxTestPalette.cpp
	QxStreamForwarder.cpp
	QxProcessObserver.cpp
	QxVideoTerminal.cpp
	EscapeSequenceSyntax.cpp
	MessageSyntax.cpp
	UriSyntax.cpp
	QxPacuTheme.cpp
	QxActivation.cpp
	QxActivationDialog.cpp
	QxAboutDialog.cpp
	QxLicenseDialog.cpp
	QxCurrentDirDialog.cpp
	LogDisplay.cpp
	InterpositionProxy.cpp
	main.cpp
)

SET(Pacu_MOC_HDRS
	QxCustomStyle.hpp
	QxWrapperStyle.hpp
	QxTabBar.hpp
	QxWidgetStack.hpp
	QxTabWidget.hpp
	QxComboBox.hpp
	QxFileSystemWatcher.hpp
	QxTerminalStack.hpp
	QxMainWindow.hpp
	QxMonospaceFilter.hpp
	QxPreferences.hpp
	QxCodetipsPreferences.hpp
	QxCommand.hpp
	QxCommandEditor.hpp
	QxStatusBar.hpp
	QxIconSelector.hpp
	QxApplication.hpp
	QxFileSystemView.hpp
	QxUrlListView.hpp
	QxFileBrowser.hpp
	QxCodeBrowser.hpp
	QxFileFinder.hpp
	QxFileSearchEdit.hpp
	QxListPopup.hpp
	QxFileSearchItem.hpp
	QxMatchView.hpp
	QxFindText.hpp
	QxStreamForwarder.hpp
	QxProcessObserver.hpp
	QxVideoTerminal.hpp
	QxActivation.hpp
	QxActivationDialog.hpp
	QxAboutDialog.hpp
	QxLicenseDialog.hpp
	QxCurrentDirDialog.hpp
	SignalAdapter.hpp
)

#SET(Pacu_UIS
#	QxCommandEditor.ui
#)

#SET(Pacu_RCCS
#	pacu.qrc
#)

QT4_WRAP_CPP(Pacu_MOC_SRCS ${Pacu_MOC_HDRS})
QT4_WRAP_UI(Pacu_UI_SRCS ${Pacu_UIS})
# QT4_ADD_RESOURCES(Pacu_RCC_SRCS ${Pacu_RCCS})

ADD_EXECUTABLE(Pacu
	${Pacu_SRCS}
	${Pacu_MOC_SRCS}
	${Pacu_UI_SRCS}
	${Pacu_RCC_SRCS}
)

INCLUDE_DIRECTORIES(
	${PROJECT_BINARY_DIR} # for including ui headers
	${QT_QTSCRIPT_INCLUDE_DIR}
#	${QT_QTSVG_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(Pacu
	${QT_LIBRARIES}
	${QT_QTSCRIPT_LIBRARIES}
#	${QT_QTSVG_LIBRARIES}
)

IF(PACU_BUILD_TARGET STREQUAL "Carbon")
	FIND_LIBRARY(CARBON NAMES Carbon)
	TARGET_LINK_LIBRARIES(Pacu ${CARBON})
ENDIF(PACU_BUILD_TARGET STREQUAL "Carbon")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "")
