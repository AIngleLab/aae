/**

 */

#include "Types.hh"
#include <iostream>
#include <string>

namespace aingle {
namespace strings {
const std::string typeToString[] = {
    "string",
    "bytes",
    "int",
    "long",
    "float",
    "double",
    "boolean",
    "null",
    "record",
    "enum",
    "array",
    "map",
    "union",
    "fixed",
    "symbolic"};

static_assert((sizeof(typeToString) / sizeof(std::string)) == (AINGLE_NUM_TYPES + 1),
              "Incorrect AIngle typeToString");

} // namespace strings

// this static assert exists because a 32 bit integer is used as a bit-flag for each type,
// and it would be a problem for this flag if we ever supported more than 32 types
static_assert(AINGLE_NUM_TYPES < 32, "Too many AIngle types");

const std::string &toString(Type type) noexcept {
    static std::string undefinedType = "Undefined type";
    if (isAIngleTypeOrPseudoType(type)) {
        return strings::typeToString[type];
    } else {
        return undefinedType;
    }
}

std::ostream &operator<<(std::ostream &os, Type type) {
    if (isAIngleTypeOrPseudoType(type)) {
        os << strings::typeToString[type];
    } else {
        os << static_cast<int>(type);
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Null &) {
    os << "(null value)";
    return os;
}

} // namespace aingle
