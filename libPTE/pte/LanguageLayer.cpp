#include "LanguageManager.hpp"
#include "SyntaxDefinition.hpp"
#include "LanguageLayer.hpp"

namespace pte
{

LanguageLayer::LanguageLayer(Ref<LanguageManager> manager, Ref<SyntaxDefinition> syntax)
	: manager_(manager),
	  syntax_(syntax),
	  styles_(new Styles(syntax_->numRules()))
{
	clearStyles();
	manager->addLayer(syntax->id(), this);
}

Ref<SyntaxDefinition> LanguageLayer::syntax() const { return syntax_; }

Ref<Style> LanguageLayer::style(const char* ruleName) const
{
	return styles_->get(syntax_->ruleByName(ruleName)->id());
}

void LanguageLayer::setStyle(const char* ruleName, QColor color, bool bold)
{
	styles_->set(syntax_->ruleByName(ruleName)->id(), new Style(color, Qt::transparent, bold));
}

void LanguageLayer::setStyle(const char* ruleName, Ref<Style> style)
{
	styles_->set(syntax_->ruleByName(ruleName)->id(), style);
}

void LanguageLayer::clearStyles() { styles_->clear(); }

} // namespace pte
