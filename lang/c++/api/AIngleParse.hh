/*

 */

#ifndef aingle_AIngleParse_hh__
#define aingle_AIngleParse_hh__

#include "AIngleTraits.hh"
#include "Config.hh"
#include "ResolvingReader.hh"

/// \file
///
/// Standalone parse functions for AIngle types.

namespace aingle {

/// The main parse entry point function.  Takes a parser (either validating or
/// plain) and the object that should receive the parsed data.

template<typename Reader, typename T>
void parse(Reader &p, T &val) {
    parse(p, val, is_serializable<T>());
}

template<typename T>
void parse(ResolvingReader &p, T &val) {
    translatingParse(p, val, is_serializable<T>());
}

/// Type trait should be set to is_serializable in otherwise force the compiler to complain.

template<typename Reader, typename T>
void parse(Reader &p, T &val, const std::false_type &) {
    static_assert(sizeof(T) == 0, "Not a valid type to parse");
}

template<typename Reader, typename T>
void translatingParse(Reader &p, T &val, const std::false_type &) {
    static_assert(sizeof(T) == 0, "Not a valid type to parse");
}

// @{

/// The remainder of the file includes default implementations for serializable types.

template<typename Reader, typename T>
void parse(Reader &p, T &val, const std::true_type &) {
    p.readValue(val);
}

template<typename Reader>
void parse(Reader &p, std::vector<uint8_t> &val, const std::true_type &) {
    p.readBytes(val);
}

template<typename T>
void translatingParse(ResolvingReader &p, T &val, const std::true_type &) {
    p.parse(val);
}

// @}

} // namespace aingle

#endif
