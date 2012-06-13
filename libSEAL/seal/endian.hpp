#include <ftl/types.hpp>

namespace seal {

using namespace ftl;

template<class UInt>
inline UInt sealEndianGate(UInt x) { return endianGate(x, LittleEndian); }

} // namespace seal
