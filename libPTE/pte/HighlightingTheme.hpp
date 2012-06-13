#ifndef PTE_HIGHLIGHTINGTHEME_HPP
#define PTE_HIGHLIGHTINGTHEME_HPP

#include "CustomList.hpp"
#include "SyntaxStyle.hpp"

namespace pte
{

class ThemeManager;
class ViewColors;

class HighlightingTheme: public Instance, public virtual BackRefList
{
public:
	HighlightingTheme();
	HighlightingTheme(Ref<ThemeManager> manager, QString name, QString displayName, Ref<HighlightingTheme> super = 0);
	~HighlightingTheme();
	
	inline QString name() const { return name_; }
	inline QString displayName() const { return displayName_; }
	
	Ref<ViewColors> viewColors() const;
	void setViewColors(Ref<ViewColors> colors);
	
private:
	friend class ThemeManager;
	friend class SyntaxStyle;
	
	typedef CustomList< Ref<SyntaxStyle> > SyntaxStyles;
	
	inline void addStyle(Ref<SyntaxStyle> style) { styles_->append(style); }
	
	Ref<HighlightingTheme, SetNull> super_;
	QString name_, displayName_;
	Ref<SyntaxStyles, Owner> styles_;
	Ref<ViewColors, Owner> viewColors_;
};

} // namespace pte

#endif // PTE_HIGHLIGHTINGTHEME_HPP
