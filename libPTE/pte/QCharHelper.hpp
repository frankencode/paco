#ifndef PTE_QCHARHELPER_HPP
#define PTE_QCHARHELPER_HPP

#include <ftl/atoms>
#include <QChar>

namespace ftl
{

/** Specialize filters for QChar, because QChar is missing +/- operators.
  * Required to make classes inherited from ftl::Syntax compile correctly.
  */

template<>
class ToLower<QChar> {
public:
	inline static QChar map(const QChar& ch) { return ch.toLower(); }
};

template<>
class ToUpper<QChar> {
public:
	inline static QChar map(const QChar& ch) { return ch.toUpper(); }
};

template<>
class ToAscii<QChar> {
public:
	inline static char map(const QChar& ch) { return char(ch.unicode()); }
};

template<>
class ToUnicode<QChar> {
public:
	inline static uchar_t map(const QChar& ch) { return uchar_t(ch.unicode()); }
};

} // namespace ftl

#endif // PTE_QCHARHELPER_HPP
