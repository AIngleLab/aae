/*

 */

#ifndef aingle_ResolvingReader_hh__
#define aingle_ResolvingReader_hh__

#include <boost/noncopyable.hpp>
#include <stdint.h>

#include "Config.hh"
#include "Reader.hh"
#include "ResolverSchema.hh"

namespace aingle {

class AINGLE_DECL ResolvingReader : private boost::noncopyable {

public:
    ResolvingReader(const ResolverSchema &schema, const InputBuffer &in) : reader_(in),
                                                                           schema_(schema) {}

    template<typename T>
    void parse(T &object) {
        schema_.parse(reader_, reinterpret_cast<uint8_t *>(&object));
    }

private:
    Reader reader_;
    ResolverSchema schema_;
};

} // namespace aingle

#endif
