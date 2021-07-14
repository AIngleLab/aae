/*

 */

#ifndef aingle_AIngleSerialize_hh__
#define aingle_AIngleSerialize_hh__

#include "AIngleTraits.hh"
#include "Config.hh"

/// \file
///
/// Standalone serialize functions for AIngle types.

namespace aingle {

/// The main serializer entry point function.  Takes a serializer (either validating or
/// plain) and the object that should be serialized.

template<typename Writer, typename T>
void serialize(Writer &s, const T &val) {
    serialize(s, val, is_serializable<T>());
}

/// Type trait should be set to is_serializable in otherwise force the compiler to complain.

template<typename Writer, typename T>
void serialize(Writer &s, const T &val, const std::false_type &) {
    static_assert(sizeof(T) == 0, "Not a valid type to serialize");
}

/// The remainder of the file includes default implementations for serializable types.

// @{

template<typename Writer, typename T>
void serialize(Writer &s, T val, const std::true_type &) {
    s.writeValue(val);
}

template<typename Writer>
void serialize(Writer &s, const std::vector<uint8_t> &val, const std::true_type &) {
    s.writeBytes(val.data(), val.size());
}

// @}

} // namespace aingle

#endif
