#ifndef PTE_SYNTAXSTYLE_HPP
#define PTE_SYNTAXSTYLE_HPP

#include <QString>
#include <QColor>
#include "UseFtl.hpp"

namespace pte
{

class HighlightingTheme;
class LanguageLayer;

class SyntaxStyle: public Instance
{
public:
	SyntaxStyle();
	SyntaxStyle(Ref<HighlightingTheme> theme, Ref<LanguageLayer> layer, const QString& ruleName, const QColor& color, bool bold);
	
	Ref<LanguageLayer, SetNull> layer_;
	QString ruleName_;
	QColor color_;
	bool bold_;
};

} // namespace pte

#endif // PTE_SYNTAXSTYLE_HPP
