#ifndef PACU_STRINGADAPTER_HPP
#define PACU_STRINGADAPTER_HPP

#include <QString>
#include <pte/QCharHelper.hpp>

namespace pacu
{

class StringAdapter
{
public:
	typedef int Index;
	typedef QChar Item;
	
	StringAdapter(QString* s)
		: s_(s),
		  len_(s->length())
	{}
	
	StringAdapter(StringAdapter* media, int len)
		: s_(media->s_),
		  len_(len)
	{
		Q_ASSERT(len <= media->s_->length());
	}
	
	static inline int ill() { return -1; }
	
	inline bool has(int i) const { return (0 <= i) && (i < len_); }
	inline Item get(int i) { return s_->at(i); }
	inline void set(int i, Item ch) { (*s_)[i] = ch; }
	
	inline int length() const { return len_; }
	
private:
	QString* s_;
	int len_;
};

} // namespace pacu

#endif // PACU_STRINGADAPTER_HPP
