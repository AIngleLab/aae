/*

 */

#ifndef aingle_AIngleTraits_hh__
#define aingle_AIngleTraits_hh__

#include "Config.hh"
#include "Types.hh"
#include <cstdint>
#include <type_traits>

/** @file
 *
 * This header contains type traits and similar utilities used by the library.
 */
namespace aingle {

/**
 * Define an is_serializable trait for types we can serialize natively.
 * New types will need to define the trait as well.
 */
template<typename T>
struct is_serializable : public std::false_type {};

template<typename T>
struct is_promotable : public std::false_type {};

template<typename T>
struct type_to_aingle {
    static const Type type = AINGLE_NUM_TYPES;
};

/**
 * Check if a \p T is a complete type i.e. it is defined as opposed to just
 * declared.
 *
 * is_defined<T>::value will be true or false depending on whether T is a
 * complete type or not respectively.
 */
template<class T>
struct is_defined {

    typedef char yes[1];

    typedef char no[2];

    template<class U>
    static yes &test(char (*)[sizeof(U)]) { throw 0; };

    template<class U>
    static no &test(...) { throw 0; };

    static const bool value = sizeof(test<T>(0)) == sizeof(yes);
};

/**
 * Similar to is_defined, but used to check if T is not defined.
 *
 * is_not_defined<T>::value will be true or false depending on whether T is an
 * incomplete type or not respectively.
 */
template<class T>
struct is_not_defined {

    typedef char yes[1];

    typedef char no[2];

    template<class U>
    static yes &test(char (*)[sizeof(U)]) { throw 0; };

    template<class U>
    static no &test(...) { throw 0; };

    static const bool value = sizeof(test<T>(0)) == sizeof(no);
};

#define DEFINE_PRIMITIVE(CTYPE, AINGLETYPE)                     \
    template<>                                                \
    struct is_serializable<CTYPE> : public std::true_type {}; \
                                                              \
    template<>                                                \
    struct type_to_aingle<CTYPE> {                              \
        static const Type type = AINGLETYPE;                    \
    };

#define DEFINE_PROMOTABLE_PRIMITIVE(CTYPE, AINGLETYPE)        \
    template<>                                              \
    struct is_promotable<CTYPE> : public std::true_type {}; \
                                                            \
    DEFINE_PRIMITIVE(CTYPE, AINGLETYPE)

DEFINE_PROMOTABLE_PRIMITIVE(int32_t, AINGLE_INT)
DEFINE_PROMOTABLE_PRIMITIVE(int64_t, AINGLE_LONG)
DEFINE_PROMOTABLE_PRIMITIVE(float, AINGLE_FLOAT)
DEFINE_PRIMITIVE(double, AINGLE_DOUBLE)
DEFINE_PRIMITIVE(bool, AINGLE_BOOL)
DEFINE_PRIMITIVE(Null, AINGLE_NULL)
DEFINE_PRIMITIVE(std::string, AINGLE_STRING)
DEFINE_PRIMITIVE(std::vector<uint8_t>, AINGLE_BYTES)

} // namespace aingle

#endif
