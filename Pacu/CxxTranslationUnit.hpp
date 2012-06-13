#ifndef PACU_CLASSSYNTAX_HPP
#define PACU_CLASSSYNTAX_HPP

namespace pacu
{

class CxxTranslationUnit: public SyntaxRule< Array<int> >
{
public:
	CxxTranslationUnit();
	
	Ref<Token, Owner> match(Array<int>* media);
};

} // namespace pacu


#endif // PACU_CLASSSYNTAX_HPP
