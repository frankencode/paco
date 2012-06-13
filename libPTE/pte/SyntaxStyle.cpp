#include "HighlightingTheme.hpp"
#include "LanguageLayer.hpp"
#include "SyntaxStyle.hpp"

namespace pte
{

SyntaxStyle::SyntaxStyle()
	: bold_(false)
{}

SyntaxStyle::SyntaxStyle(Ref<HighlightingTheme> theme, Ref<LanguageLayer> layer, const QString& ruleName, const QColor& color, bool bold)
	: layer_(layer),
	  ruleName_(ruleName),
	  color_(color),
	  bold_(bold)
{
	theme->addStyle(this);
}

} // namespace pte
