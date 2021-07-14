/*

 */

#ifndef aingle_Types_hh__
#define aingle_Types_hh__

#include <iostream>

#include "Config.hh"

namespace aingle {

/**
 * The "type" for the schema.
 */
enum Type {

    AINGLE_STRING, /*!< String */
    AINGLE_BYTES,  /*!< Sequence of variable length bytes data */
    AINGLE_INT,    /*!< 32-bit integer */
    AINGLE_LONG,   /*!< 64-bit integer */
    AINGLE_FLOAT,  /*!< Floating point number */
    AINGLE_DOUBLE, /*!< Double precision floating point number */
    AINGLE_BOOL,   /*!< Boolean value */
    AINGLE_NULL,   /*!< Null */

    AINGLE_RECORD, /*!< Record, a sequence of fields */
    AINGLE_ENUM,   /*!< Enumeration */
    AINGLE_ARRAY,  /*!< Homogeneous array of some specific type */
    AINGLE_MAP,    /*!< Homogeneous map from string to some specific type */
    AINGLE_UNION,  /*!< Union of one or more types */
    AINGLE_FIXED,  /*!< Fixed number of bytes */

    AINGLE_NUM_TYPES, /*!< Marker */

    // The following is a pseudo-type used in implementation

    AINGLE_SYMBOLIC = AINGLE_NUM_TYPES, /*!< User internally to avoid circular references. */
    AINGLE_UNKNOWN = -1               /*!< Used internally. */
};

/**
 * Returns true if and only if the given type is a primitive.
 * Primitive types are: string, bytes, int, long, float, double, boolean
 * and null
 */
inline constexpr bool isPrimitive(Type t) noexcept {
    return (t >= AINGLE_STRING) && (t < AINGLE_RECORD);
}

/**
 * Returns true if and only if the given type is a non primitive valid type.
 * Primitive types are: string, bytes, int, long, float, double, boolean
 * and null
 */
inline constexpr bool isCompound(Type t) noexcept {
    return (t >= AINGLE_RECORD) && (t < AINGLE_NUM_TYPES);
}

/**
 * Returns true if and only if the given type is a valid aingle type.
 */
inline constexpr bool isAIngleType(Type t) noexcept {
    return (t >= AINGLE_STRING) && (t < AINGLE_NUM_TYPES);
}

/**
 * Returns true if and only if the given type is within the valid range
 * of enumeration.
 */
inline constexpr bool isAIngleTypeOrPseudoType(Type t) noexcept {
    return (t >= AINGLE_STRING) && (t <= AINGLE_NUM_TYPES);
}

/**
 * Converts the given type into a string. Useful for generating messages.
 */
AINGLE_DECL const std::string &toString(Type type) noexcept;

/**
 * Writes a string form of the given type into the given ostream.
 */
AINGLE_DECL std::ostream &operator<<(std::ostream &os, aingle::Type type);

/// define a type to represent AIngle Null in template functions
struct AINGLE_DECL Null {};

/**
 * Writes schema for null \p null type to \p os.
 * \param os The ostream to write to.
 * \param null The value to be written.
 */
std::ostream &operator<<(std::ostream &os, const Null &null);

} // namespace aingle

#endif
