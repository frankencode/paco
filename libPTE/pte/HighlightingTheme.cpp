#include "ViewColors.hpp"
#include "ThemeManager.hpp"
#include "HighlightingTheme.hpp"

namespace pte
{

HighlightingTheme::HighlightingTheme()
{}

HighlightingTheme::HighlightingTheme(Ref<ThemeManager> manager, QString name, QString displayName, Ref<HighlightingTheme> super)
	: super_(super),
	  name_(name),
	  displayName_(displayName),
	  styles_(new SyntaxStyles)
{
	manager->addTheme(this);
}

HighlightingTheme::~HighlightingTheme()
{}

Ref<ViewColors> HighlightingTheme::viewColors() const
{
	return viewColors_;
}

void HighlightingTheme::setViewColors(Ref<ViewColors> colors)
{
	viewColors_ = colors;
}


} // namespace pte
