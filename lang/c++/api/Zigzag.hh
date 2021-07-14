/*

 */

#ifndef aingle_Encoding_hh__
#define aingle_Encoding_hh__

#include <array>
#include <cstddef>
#include <cstdint>

#include "Config.hh"
/// \file
/// Functions for encoding and decoding integers with zigzag compression

namespace aingle {

AINGLE_DECL constexpr uint64_t encodeZigzag64(int64_t input) noexcept {
    // cppcheck-suppress shiftTooManyBitsSigned
    return ((input << 1) ^ (input >> 63));
}
AINGLE_DECL constexpr int64_t decodeZigzag64(uint64_t input) noexcept {
    return static_cast<int64_t>(((input >> 1) ^ -(static_cast<int64_t>(input) & 1)));
}

AINGLE_DECL constexpr uint32_t encodeZigzag32(int32_t input) noexcept {
    // cppcheck-suppress shiftTooManyBitsSigned
    return ((input << 1) ^ (input >> 31));
}
AINGLE_DECL constexpr int32_t decodeZigzag32(uint32_t input) noexcept {
    return static_cast<int32_t>(((input >> 1) ^ -(static_cast<int64_t>(input) & 1)));
}

AINGLE_DECL size_t encodeInt32(int32_t input, std::array<uint8_t, 5> &output) noexcept;
AINGLE_DECL size_t encodeInt64(int64_t input, std::array<uint8_t, 10> &output) noexcept;

} // namespace aingle

#endif
