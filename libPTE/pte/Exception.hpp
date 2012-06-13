#ifndef PTE_EXCEPTION_HPP
#define PTE_EXCEPTION_HPP

#include "UseFtl.hpp"

namespace pte
{

#define PTE_EXCEPTION(DerivedClass, BaseClass) FTL_EXCEPTION(DerivedClass, BaseClass)

#define PTE_THROW(Exception, reason) FTL_THROW(Exception, QString(reason).toUtf8().constData())

} // namespace pte

#endif // PTE_EXCEPTION_HPP
