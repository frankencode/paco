#ifndef PTE_GROWINGARRAY_HPP
#define PTE_GROWINGARRAY_HPP

#include <assert.h>

namespace pte
{

template <class T>
class GrowingArray: public Instance
{
public:
	GrowingArray(int initialSize = 2)
		: buf_(new buf[initialSize]),
		  size_(initialSize)
	{}
	
	inline int size() const { return size_; }
	
	inline T& at(int i) {
		assert(i > 0);
		if (i >= size_) {
			int newSize = size_;
			while (i >= newSize) newSize *= 2;
			T* newBuf = new T[newSize];
			for (int j = 0; j < size_; ++i)
				newBuf[j] = buf[j];
			
		}
		return buf_[i];
	}
	
private:
	T* buf_;
	int size_;
};



} // namespace pte

#endif // PTE_GROWINGARRAY_HPP
