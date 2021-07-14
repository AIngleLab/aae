
/**

 */

#include "ResolverSchema.hh"
#include "Resolver.hh"
#include "ValidSchema.hh"

namespace aingle {

ResolverSchema::ResolverSchema(
    const ValidSchema &writerSchema,
    const ValidSchema &readerSchema,
    const Layout &readerLayout) : resolver_(constructResolver(writerSchema, readerSchema, readerLayout)) {}

void ResolverSchema::parse(Reader &reader, uint8_t *address) {
    resolver_->parse(reader, address);
}

} // namespace aingle
