#ifndef PTE_SYNTAXDEFINITION_HPP
#define PTE_SYNTAXDEFINITION_HPP

// #define PTE_DEBUG_SYNTAX

#include <ftl/Syntax.hpp>
#include <ftl/SyntaxDebugger.hpp>
#include "UseFtl.hpp"
#include "SequenceAdapter.hpp"

namespace pte
{

typedef Syntax<SequenceAdapter>::Definition SyntaxDefinitionSuper;
typedef SyntaxDebugger<SequenceAdapter> SyntaxDebugger;

class SyntaxDefinition: public SyntaxDefinitionSuper
{
public:
	SyntaxDefinition(Ref<Scope> scope, const char* name)
		: SyntaxDefinitionSuper(
		  	#ifdef PTE_DEBUG_SYNTAX
		  	new SyntaxDebugger
		  	#else
		  	/*debugFactory=*/0
		  	#endif
		  	, scope, name
		  )
		  #ifdef PTE_DEBUG_SYNTAX
		  , debugger_(debugFactory())
		  #endif
	{}
	
	#ifdef PTE_DEBUG_SYNTAX
	inline Ref<SyntaxDebugger> debugger() const { return debugger_; }
	#endif
	
private:
	#ifdef PTE_DEBUG_SYNTAX
	Ref<SyntaxDebugger, Owner> debugger_;
	#endif
};

} // namespace pte

#endif // PTE_SYNTAXDEFINITION_HPP
