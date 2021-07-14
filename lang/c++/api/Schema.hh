/*

 */

#ifndef aingle_Schema_hh__
#define aingle_Schema_hh__

#include "Config.hh"
#include "NodeImpl.hh"
#include <string>

/// \file
///
/// Schemas for representing all the aingle types.  The compound schema objects
/// allow composition from other schemas.
///

namespace aingle {

/// The root Schema object is a base class.  Nobody constructs this class directly.

class AINGLE_DECL Schema {
public:
    virtual ~Schema() = default;

    Type type() const {
        return node_->type();
    }

    const NodePtr &root() const {
        return node_;
    }

    NodePtr &root() {
        return node_;
    }

protected:
    explicit Schema(NodePtr node) : node_(std::move(node)) {}
    explicit Schema(Node *node) : node_(node) {}

    NodePtr node_;
};

class AINGLE_DECL NullSchema : public Schema {
public:
    NullSchema() : Schema(new NodePrimitive(AINGLE_NULL)) {}
};

class AINGLE_DECL BoolSchema : public Schema {
public:
    BoolSchema() : Schema(new NodePrimitive(AINGLE_BOOL)) {}
};

class AINGLE_DECL IntSchema : public Schema {
public:
    IntSchema() : Schema(new NodePrimitive(AINGLE_INT)) {}
};

class AINGLE_DECL LongSchema : public Schema {
public:
    LongSchema() : Schema(new NodePrimitive(AINGLE_LONG)) {}
};

class AINGLE_DECL FloatSchema : public Schema {
public:
    FloatSchema() : Schema(new NodePrimitive(AINGLE_FLOAT)) {}
};

class AINGLE_DECL DoubleSchema : public Schema {
public:
    DoubleSchema() : Schema(new NodePrimitive(AINGLE_DOUBLE)) {}
};

class AINGLE_DECL StringSchema : public Schema {
public:
    StringSchema() : Schema(new NodePrimitive(AINGLE_STRING)) {}
};

class AINGLE_DECL BytesSchema : public Schema {
public:
    BytesSchema() : Schema(new NodePrimitive(AINGLE_BYTES)) {}
};

class AINGLE_DECL RecordSchema : public Schema {
public:
    explicit RecordSchema(const std::string &name);
    void addField(const std::string &name, const Schema &fieldSchema);

    std::string getDoc() const;
    void setDoc(const std::string &);
};

class AINGLE_DECL EnumSchema : public Schema {
public:
    explicit EnumSchema(const std::string &name);
    void addSymbol(const std::string &symbol);
};

class AINGLE_DECL ArraySchema : public Schema {
public:
    explicit ArraySchema(const Schema &itemsSchema);
    ArraySchema(const ArraySchema &itemsSchema);
};

class AINGLE_DECL MapSchema : public Schema {
public:
    explicit MapSchema(const Schema &valuesSchema);
    MapSchema(const MapSchema &itemsSchema);
};

class AINGLE_DECL UnionSchema : public Schema {
public:
    UnionSchema();
    void addType(const Schema &typeSchema);
};

class AINGLE_DECL FixedSchema : public Schema {
public:
    FixedSchema(int size, const std::string &name);
};

class AINGLE_DECL SymbolicSchema : public Schema {
public:
    SymbolicSchema(const Name &name, const NodePtr &link);
};
} // namespace aingle

#endif
