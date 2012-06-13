#ifndef PTE_VIEWMETRICS_HPP
#define PTE_VIEWMETRICS_HPP

#include <QFont>
#include "ViewColors.hpp"

namespace pte
{

class ViewMetrics: public Instance
{
public:
	ViewMetrics();
	
	QFont font_;
	bool fontAntialiasing_;
	int subpixelAntialiasing_;
	int blanking_; // inside margins
	int wrapMode_;
	int cursorStyle_;
	int cursorWidth_;
	bool showWhitespace_;
	bool showLineNumbers_;
	int lineSpacing_;
	int tabWidth_;
	int selectionAutoScrollTimeout_;
	bool kinematicScrolling_;
	int scrollingSpeed_;
};

} // namespace pte

#endif // PTE_VIEWMETRICS_HPP
