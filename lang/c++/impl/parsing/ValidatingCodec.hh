/**

 */

#ifndef aingle_parsing_ValidatingCodec_hh__
#define aingle_parsing_ValidatingCodec_hh__

#include <map>
#include <vector>

#include "NodeImpl.hh"
#include "Symbol.hh"
#include "ValidSchema.hh"

namespace aingle {
namespace parsing {

class ValidatingGrammarGenerator {
protected:
    virtual ProductionPtr doGenerate(const NodePtr &n,
                                     std::map<NodePtr, ProductionPtr> &m);

    ProductionPtr generate(const NodePtr &schema);

public:
    Symbol generate(const ValidSchema &schema);
};

} // namespace parsing
} // namespace aingle

#endif
