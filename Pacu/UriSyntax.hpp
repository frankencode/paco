#ifndef PACU_URISYNTAX_HPP
#define PACU_URISYNTAX_HPP

#include <ftl/Syntax.hpp>
#include "StringAdapter.hpp"
#include "UseFtl.hpp"

namespace pacu
{

class UriSyntax: public Syntax<StringAdapter>::Definition
{
public:
	UriSyntax();
};

} // namespace pacu

#endif // PACU_URISYNTAX_HPP
