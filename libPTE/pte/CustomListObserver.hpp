#ifndef PTE_CUSTOMLISTOBSERVER_HPP
#define PTE_CUSTOMLISTOBSERVER_HPP

#include "UseFtl.hpp"

namespace pte
{

template<class T>
class CustomListObserver: public virtual Instance
{
public:
	virtual bool beforePush(int i, int n, const T* v) { return true; }
	virtual void afterPush(int i, int n) {}
	
	virtual bool beforePop(int i, int n) { return true; }
	virtual void afterPop(int i, int n) {}
	
	virtual bool beforeWrite(int i, int n, const T* v) { return true; }
	virtual void afterWrite(int i, int n) {}
};

} // namespace pte

#endif // PTE_CUSTOMLISTOBSERVER_HPP
