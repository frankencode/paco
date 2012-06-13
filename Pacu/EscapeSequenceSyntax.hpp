#ifndef PACU_ESCAPESEQUENCESYNTAX_HPP
#define PACU_ESCAPESEQUENCESYNTAX_HPP

#include <ftl/Syntax.hpp>
#include "UseFtl.hpp"

namespace pacu
{

class EscapeSequenceSyntax: public Syntax<Array<uint8_t, ShallowCopyArray> >::Definition
{
public:
	EscapeSequenceSyntax();
	
	int number_;
	int title_;
	int controlSequence_;
};

} // namespace pacu

#endif // PACU_ESCAPESEQUENCESYNTAX_HPP
