#ifndef PTE_LANGUAGELAYER_HPP
#define PTE_LANGUAGELAYER_HPP

#include <ftl/Array.hpp>
#include "UseFtl.hpp"
#include "Atoms.hpp"

namespace pte
{

class LanguageManager;
class SyntaxDefinition;

class LanguageLayer: public Instance, public virtual BackRefList
{
public:
	LanguageLayer(Ref<LanguageManager> manager, Ref<SyntaxDefinition> documentSyntax);
	
	Ref<SyntaxDefinition> syntax() const;
	
	inline Ref<Style> style(int rule) const { return styles_->get(rule); }
	Ref<Style> style(const char* ruleName) const;
	void setStyle(const char* ruleName, QColor color, bool bold = false);
	void setStyle(const char* ruleName, Ref<Style> style);
	void clearStyles();
	
private:
	Ref<LanguageManager, SetNull> manager_;
	Ref<SyntaxDefinition, Owner> syntax_;
	
	typedef Array< Ref<Style, Owner> > Styles;
	Ref<Styles, Owner> styles_;
};

} // namespace pte

#endif // PTE_LANGUAGELAYER_HPP
