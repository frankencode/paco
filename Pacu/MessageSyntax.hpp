#ifndef PACU_MESSAGESYNTAX_HPP
#define PACU_MESSAGESYNTAX_HPP

#include <ftl/Syntax.hpp>
#include "StringAdapter.hpp"
#include "UseFtl.hpp"

namespace pacu
{

class MessageSyntax: public Syntax<StringAdapter>::Definition
{
public:
	MessageSyntax();
	
	int path_;
	int number_;
	int row_;
	int col_;
	int type_;
	int pattern_;
	int text_;
	int message_;
};

} // namespace pacu

#endif // PACU_MESSAGESYNTAX_HPP
