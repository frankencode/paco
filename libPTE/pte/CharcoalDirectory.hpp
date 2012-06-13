#ifndef PTE_CHARCOALDIRECTORY_HPP
#define PTE_CHARCOALDIRECTORY_HPP

#include <QPixmap>
#include <ftl/Path.hpp>
#include "ScriptableSyntaxDefinition.hpp"

class QScriptEngine;

namespace pte
{

class ViewColors;
class LanguageManager;
class ThemeManager;
class PaletteManager;
class HighlightingTheme;

class CharcoalDirectory: public QObject, public Instance
{
public:
	CharcoalDirectory();
	
	Ref<LanguageManager> languageManager() const;
	Ref<ThemeManager> themeManager() const;
	Ref<PaletteManager> paletteManager() const;
	
	QPixmap themePreview(Ref<HighlightingTheme> theme) const;
	
private:
	static String findCharcoalDir();
	QString readText(String path);
	void checkError(String path);
	void readScript(String path);
	void evalSyntax();
	void readStack(QString stackName, QScriptValue stackValue);
	void evalStacks();
	void evalThemes();
	void evalColors();
	
	QScriptEngine* engine_;
	Path path_;
	
	Ref<LanguageManager, Owner> languageManager_;
	Ref<ThemeManager, Owner> themeManager_;
	Ref<PaletteManager, Owner> paletteManager_;
};

} // namespace pte

#endif // PTE_CHARCOALDIRECTORY_HPP
