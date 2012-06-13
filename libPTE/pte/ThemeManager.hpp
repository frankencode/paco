#ifndef PTE_THEMEMANAGER_HPP
#define PTE_THEMEMANAGER_HPP

#include <QObject>
#include <ftl/Map.hpp>
#include <ftl/List.hpp>
#include "UseFtl.hpp"
#include "CustomList.hpp"
#include "ViewColors.hpp"
#include "SyntaxStyle.hpp"
#include "HighlightingTheme.hpp"

namespace pte
{

class LanguageManager;

class ThemeManager: public QObject, public Instance
{
	Q_OBJECT
	
public:
	ThemeManager(Ref<LanguageManager> languageManager);
	~ThemeManager();
	
	Ref<HighlightingTheme> themeByName(QString name) const;
	Ref<HighlightingTheme> themeByIndex(int index) const;
	int numThemes() const;
	
	int themeIndex(QString name) const;
	int defaultThemeIndex() const;
	Ref<HighlightingTheme> activeTheme() const;
	
public slots:
	void activateThemeByIndex(int index);
	void activateThemeByIndexSilent(int index);
	
signals:
	void highlightersYield();
	void viewColorsChanged(Ref<ViewColors> colors);
	void highlightersRestart();
	void highlightersSync();
	void highlightersResume();
	
private:
	friend class CharcoalDirectory;
	friend class HighlightingTheme;
	
	void activateTheme(Ref<HighlightingTheme> theme);
	
	void addTheme(Ref<HighlightingTheme> theme);
	void sortThemesByDisplayName();
	
	Ref<LanguageManager, SetNull> languageManager_;
	
	typedef Map< QString, Ref<HighlightingTheme, Owner> > ThemesByName;
	typedef List< Ref<HighlightingTheme, Owner> > ThemesByIndex;
	Ref<ThemesByName, Owner> themesByName_;
	Ref<ThemesByIndex, Owner> themesByIndex_;
	Ref<HighlightingTheme, Owner> activeTheme_;
};

} // namespace pte

#endif // PTE_THEMEMANAGER_HPP
