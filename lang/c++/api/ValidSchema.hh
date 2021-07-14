/*

 */

#ifndef aingle_ValidSchema_hh__
#define aingle_ValidSchema_hh__

#include "Config.hh"
#include "Node.hh"

namespace aingle {

class AINGLE_DECL Schema;

/// A ValidSchema is basically a non-mutable Schema that has passed some
/// minimum of sanity checks.  Once validated, any Schema that is part of
/// this ValidSchema is considered locked, and cannot be modified (an attempt
/// to modify a locked Schema will throw).  Also, as it is validated, any
/// recursive duplications of schemas are replaced with symbolic links to the
/// original.
///
/// Once a Schema is converted to a valid schema it can be used in validating
/// parsers/serializers, converted to a json schema, etc.
///

class AINGLE_DECL ValidSchema {
public:
    explicit ValidSchema(NodePtr root);
    explicit ValidSchema(const Schema &schema);
    ValidSchema();

    void setSchema(const Schema &schema);

    const NodePtr &root() const {
        return root_;
    }

    void toJson(std::ostream &os) const;
    std::string toJson(bool prettyPrint = true) const;

    void toFlatList(std::ostream &os) const;

protected:
    NodePtr root_;

private:
    static std::string compactSchema(const std::string &schema);
};

} // namespace aingle

#endif
