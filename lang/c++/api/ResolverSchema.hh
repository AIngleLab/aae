/*

 */

#ifndef aingle_ResolverSchema_hh__
#define aingle_ResolverSchema_hh__

#include <boost/noncopyable.hpp>
#include <cstdint>
#include <memory>

#include "Config.hh"
#include "Reader.hh"

/// \file ResolverSchema.hh
///

namespace aingle {

class ValidSchema;
class Layout;
class Resolver;

class AINGLE_DECL ResolverSchema {
public:
    ResolverSchema(const ValidSchema &writer, const ValidSchema &reader, const Layout &readerLayout);

private:
    friend class ResolvingReader;
    void parse(Reader &reader, uint8_t *address);
    std::shared_ptr<Resolver> resolver_;
};

} // namespace aingle

#endif
