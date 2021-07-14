/*

 */

#ifndef aingle_SchemaResolution_hh__
#define aingle_SchemaResolution_hh__

#include "Config.hh"

namespace aingle {

enum SchemaResolution {

    /// The schemas definitely do not match

    RESOLVE_NO_MATCH,

    /// The schemas match at a cursory level
    ///
    /// For records and enums, this means the name is the same, but it does not
    /// necessarily mean that every symbol or field is an exact match.

    RESOLVE_MATCH,

    /// For primitives, the matching may occur if the type is promotable.  This means that the
    /// writer matches reader if the writer's type is promoted the specified type.

    //@{

    RESOLVE_PROMOTABLE_TO_LONG,
    RESOLVE_PROMOTABLE_TO_FLOAT,
    RESOLVE_PROMOTABLE_TO_DOUBLE,

    //@}

};

} // namespace aingle

#endif
