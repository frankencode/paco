#include "ViewColors.hpp"
#include "LanguageLayer.hpp"
#include "LanguageManager.hpp"
#include "ThemeManager.hpp"

namespace pte
{

ThemeManager::ThemeManager(Ref<LanguageManager> languageManager)
	: languageManager_(languageManager),
	  themesByName_(new ThemesByName),
	  themesByIndex_(new ThemesByIndex)
{}

ThemeManager::~ThemeManager()
{}

Ref<HighlightingTheme> ThemeManager::themeByName(QString name) const
{
	return themesByName_->value(name);
}

Ref<HighlightingTheme> ThemeManager::themeByIndex(int index) const
{
	return themesByIndex_->at(index);
}

int ThemeManager::numThemes() const
{
	return themesByIndex_->length();
}

int ThemeManager::themeIndex(QString name) const
{
	int index = -1;
	for (int i = 0, n = numThemes(); (i < n) && (index == -1); ++i)
		if (themeByIndex(i)->name() == name)
			index = i;
	return (index != -1) ? index : defaultThemeIndex();
}

int ThemeManager::defaultThemeIndex() const
{
	return themeIndex("Default");
}

Ref<HighlightingTheme> ThemeManager::activeTheme() const
{
	return activeTheme_;
}

void ThemeManager::activateThemeByIndex(int index)
{
	emit highlightersYield();
	activateThemeByIndexSilent(index);
	emit highlightersSync();
	emit highlightersResume();
}

void ThemeManager::activateThemeByIndexSilent(int index)
{
	activateTheme(themeByIndex(index));
	if (activeTheme_->viewColors())
		emit viewColorsChanged(activeTheme_->viewColors());
}

void ThemeManager::activateTheme(Ref<HighlightingTheme> theme)
{
	for (int i = 0, n = languageManager_->numLayers(); i < n; ++i)
		languageManager_->layerByIndex(i)->clearStyles();
	
	activeTheme_ = theme;
	
	if (activeTheme_)
		for (int i = 0, n = activeTheme_->styles_->length(); i < n; ++i) {
			Ref<SyntaxStyle> style = activeTheme_->styles_->get(i);
			style->layer_->setStyle(style->ruleName_.toUtf8(), style->color_, style->bold_);
		}
}

void ThemeManager::addTheme(Ref<HighlightingTheme> theme)
{
	themesByName_->insert(theme->name(), theme);
	themesByIndex_->append(theme);
}

void ThemeManager::sortThemesByDisplayName()
{
	typedef ThemesByName ThemesByDisplayName;
	Ref<ThemesByDisplayName, Owner> themesByDisplayName = new ThemesByDisplayName;
	for (int i = 0; themesByIndex_->has(i); ++i) {
		Ref<HighlightingTheme> theme = themesByIndex_->at(i);
		themesByDisplayName->insert(theme->displayName(), theme);
	}
	themesByIndex_ = new ThemesByIndex;
	for (int i = 0; i < themesByDisplayName->length(); ++i)
		themesByIndex_->append(themesByDisplayName->get(i).value());
}

} // namespace pte
