#ifndef PTE_VIEWCOLORS_HPP
#define PTE_VIEWCOLORS_HPP

#include <QColor>
#include "UseFtl.hpp"

namespace pte
{

class ViewColors: public Instance
{
public:
	ViewColors()
	  : foregroundColor_(Qt::black),
	    backgroundColor_(Qt::white),
 	    cursorColor_(Qt::black),
 	    lineNumbersFgColor_(QColor::fromRgbF(0.5, 0.5, 0.5)),
	    lineNumbersFgColorBright_(QColor::fromRgbF(0., 0., 0.)),
	    lineNumbersBgColor_(QColor::fromRgbF(0.95, 0.95, 0.95)),
	    lineNumbersBgColorBright_(QColor::fromRgbF(0.90, 0.90, 0.90)),
	    selectionColor_(QColor::fromRgbF(0.8, 0.9, 1.)),
	    matchColor_(QColor::fromRgbF(1., 1., 0.4)),
	    bracketMatchColor_(QColor::fromRgbF(1., 1., 0.3)),
	    currentLineColor_(QColor::fromRgbF(0.95, 0.95, 0.95))
	{}
	QColor foregroundColor_;
	QColor backgroundColor_;
	QColor cursorColor_;
	QColor lineNumbersFgColor_;
	QColor lineNumbersFgColorBright_;
	QColor lineNumbersBgColor_;
	QColor lineNumbersBgColorBright_;
	QColor selectionColor_;
	QColor matchColor_;
	QColor bracketMatchColor_;
	QColor currentLineColor_;
};

} // namespace pte

#endif // PTE_VIEWCOLORS_HPP
