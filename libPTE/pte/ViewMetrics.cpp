#include <QApplication>
#include "View.hpp"
#include "ViewMetrics.hpp"

namespace pte
{

ViewMetrics::ViewMetrics()
	: fontAntialiasing_(true),
#ifdef Q_WS_MAC
	  subpixelAntialiasing_(true),
#else
	  subpixelAntialiasing_(false),
#endif
	  blanking_(4),
	  wrapMode_(View::FixedColumnWrap),
	  cursorStyle_(View::NoCursor),
	  cursorWidth_(2),
	  showWhitespace_(true),
	  showLineNumbers_(true),
	  lineSpacing_(1),
	  tabWidth_(4),
	  selectionAutoScrollTimeout_(50),
	  kinematicScrolling_(true),
	  scrollingSpeed_(qApp->wheelScrollLines())
{
#ifdef Q_WS_MAC
	font_ = QFont("Monaco", 12);
	font_.setPixelSize(12);
#else
	#ifndef Q_WS_WIN
		font_ = QFont("DejaVu Sans Mono", 10);
	#else
		font_ = QFont("Console", 10); // Courier
	#endif
	font_.setPixelSize(13);
#endif
}

} // namespace pte
