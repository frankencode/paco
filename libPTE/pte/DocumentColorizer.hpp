#ifndef PTE_DOCUMENTCOLORIZER_HPP
#define PTE_DOCUMENTCOLORIZER_HPP

#include <ftl/Token.hpp>
#include <ftl/TokenScreen.hpp>
#include "UseFtl.hpp"

namespace pte
{

class LanguageManager;
class LanguageStack;
class Document;
class Line;

class DocumentColorizer: public TokenScreen
{
public:
	DocumentColorizer(Ref<LanguageStack> languageStack, Ref<Document> document, int y = -1);
	
	inline bool needUpdate() const { return needUpdate_; }
	
private:
	virtual bool project(Ref<Token> token, int i0, int i1);
	bool nextLine();
	const QString& text();
	void appendSpan(int s0, int sn, Ref<pte::Style> style, Ref<Token> token);
	
	Ref<LanguageManager> languageManager_;
	Ref<LanguageStack> languageStack_;
	Ref<Document> document_;
	Ref<Highlighter> highlighter_;
	Ref<Line> line_;
	QString text_; // current line text
	int y_; // current line number
	int ci_; // current chunk index
	int k0_, k1_; // index range of current line
	bool needUpdate_; // colors have changed
	bool singleLine_;
};

} // namespace pte

#endif // PTE_DOCUMENTCOLORIZER_HPP
