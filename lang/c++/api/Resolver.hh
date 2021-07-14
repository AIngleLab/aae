/*

 */

#ifndef aingle_Resolver_hh__
#define aingle_Resolver_hh__

#include <boost/noncopyable.hpp>
#include <cstdint>
#include <memory>

#include "Config.hh"
#include "Reader.hh"

/// \file Resolver.hh
///

namespace aingle {

class ValidSchema;
class Layout;

class AINGLE_DECL Resolver : private boost::noncopyable {
public:
    virtual void parse(Reader &reader, uint8_t *address) const = 0;
    virtual ~Resolver() = default;
};

std::unique_ptr<Resolver> constructResolver(
    const ValidSchema &writerSchema,
    const ValidSchema &readerSchema,
    const Layout &readerLayout);

} // namespace aingle

#endif
