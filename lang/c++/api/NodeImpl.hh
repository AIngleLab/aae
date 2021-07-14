/*

 */

#ifndef aingle_NodeImpl_hh__
#define aingle_NodeImpl_hh__

#include "Config.hh"
#include "GenericDatum.hh"

#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include "Node.hh"
#include "NodeConcepts.hh"

namespace aingle {

/// Implementation details for Node.  NodeImpl represents all the aingle types,
/// whose properties are enabled and disabled by selecting concept classes.

template<
    class NameConcept,
    class LeavesConcept,
    class LeafNamesConcept,
    class SizeConcept>
class NodeImpl : public Node {

protected:
    explicit NodeImpl(Type type) : Node(type),
                                   nameAttribute_(),
                                   docAttribute_(),
                                   leafAttributes_(),
                                   leafNameAttributes_(),
                                   sizeAttribute_() {}

    NodeImpl(Type type,
             const NameConcept &name,
             const LeavesConcept &leaves,
             const LeafNamesConcept &leafNames,
             const SizeConcept &size) : Node(type),
                                        nameAttribute_(name),
                                        docAttribute_(),
                                        leafAttributes_(leaves),
                                        leafNameAttributes_(leafNames),
                                        sizeAttribute_(size) {}

    // Ctor with "doc"
    NodeImpl(Type type,
             const NameConcept &name,
             const concepts::SingleAttribute<std::string> &doc,
             const LeavesConcept &leaves,
             const LeafNamesConcept &leafNames,
             const SizeConcept &size) : Node(type),
                                        nameAttribute_(name),
                                        docAttribute_(doc),
                                        leafAttributes_(leaves),
                                        leafNameAttributes_(leafNames),
                                        sizeAttribute_(size) {}

    void swap(NodeImpl &impl) {
        std::swap(nameAttribute_, impl.nameAttribute_);
        std::swap(docAttribute_, impl.docAttribute_);
        std::swap(leafAttributes_, impl.leafAttributes_);
        std::swap(leafNameAttributes_, impl.leafNameAttributes_);
        std::swap(sizeAttribute_, impl.sizeAttribute_);
        std::swap(nameIndex_, impl.nameIndex_);
    }

    bool hasName() const override {
        // e.g.: true for single and multi-attributes, false for no-attributes.
        return NameConcept::hasAttribute;
    }

    void doSetName(const Name &name) override {
        nameAttribute_.add(name);
    }

    const Name &name() const override {
        return nameAttribute_.get();
    }

    void doSetDoc(const std::string &doc) override {
        docAttribute_.add(doc);
    }

    const std::string &getDoc() const override {
        return docAttribute_.get();
    }

    void doAddLeaf(const NodePtr &newLeaf) final {
        leafAttributes_.add(newLeaf);
    }

    size_t leaves() const override {
        return leafAttributes_.size();
    }

    const NodePtr &leafAt(size_t index) const override {
        return leafAttributes_.get(index);
    }

    void doAddName(const std::string &name) override {
        if (!nameIndex_.add(name, leafNameAttributes_.size())) {
            throw Exception(boost::format("Cannot add duplicate name: %1%") % name);
        }
        leafNameAttributes_.add(name);
    }

    size_t names() const override {
        return leafNameAttributes_.size();
    }

    const std::string &nameAt(size_t index) const override {
        return leafNameAttributes_.get(index);
    }

    bool nameIndex(const std::string &name, size_t &index) const override {
        return nameIndex_.lookup(name, index);
    }

    void doSetFixedSize(size_t size) override {
        sizeAttribute_.add(size);
    }

    size_t fixedSize() const override {
        return sizeAttribute_.get();
    }

    bool isValid() const override = 0;

    void printBasicInfo(std::ostream &os) const override;

    void setLeafToSymbolic(size_t index, const NodePtr &node) override;

    SchemaResolution furtherResolution(const Node &reader) const {
        SchemaResolution match = RESOLVE_NO_MATCH;

        if (reader.type() == AINGLE_SYMBOLIC) {

            // resolve the symbolic type, and check again
            const NodePtr &node = reader.leafAt(0);
            match = resolve(*node);
        } else if (reader.type() == AINGLE_UNION) {

            // in this case, need to see if there is an exact match for the
            // writer's type, or if not, the first one that can be promoted to a
            // match

            for (size_t i = 0; i < reader.leaves(); ++i) {

                const NodePtr &node = reader.leafAt(i);
                SchemaResolution thisMatch = resolve(*node);

                // if matched then the search is done
                if (thisMatch == RESOLVE_MATCH) {
                    match = thisMatch;
                    break;
                }

                // thisMatch is either no match, or promotable, this will set match to
                // promotable if it hasn't been set already
                if (match == RESOLVE_NO_MATCH) {
                    match = thisMatch;
                }
            }
        }

        return match;
    }

    NameConcept nameAttribute_;

    // Rem: NameConcept type is HasName (= SingleAttribute<Name>), we use std::string instead
    concepts::SingleAttribute<std::string> docAttribute_; /** Doc used to compare schemas */

    LeavesConcept leafAttributes_;
    LeafNamesConcept leafNameAttributes_;
    SizeConcept sizeAttribute_;
    concepts::NameIndexConcept<LeafNamesConcept> nameIndex_;
};

using NoName = concepts::NoAttribute<Name>;
using HasName = concepts::SingleAttribute<Name>;

using HasDoc = concepts::SingleAttribute<std::string>;

using NoLeaves = concepts::NoAttribute<NodePtr>;
using SingleLeaf = concepts::SingleAttribute<NodePtr>;
using MultiLeaves = concepts::MultiAttribute<NodePtr>;

using NoLeafNames = concepts::NoAttribute<std::string>;
using LeafNames = concepts::MultiAttribute<std::string>;

using NoSize = concepts::NoAttribute<int>;
using HasSize = concepts::SingleAttribute<int>;

using NodeImplPrimitive = NodeImpl<NoName, NoLeaves, NoLeafNames, NoSize>;
using NodeImplSymbolic = NodeImpl<HasName, NoLeaves, NoLeafNames, NoSize>;

using NodeImplRecord = NodeImpl<HasName, MultiLeaves, LeafNames, NoSize>;
using NodeImplEnum = NodeImpl<HasName, NoLeaves, LeafNames, NoSize>;
using NodeImplArray = NodeImpl<NoName, SingleLeaf, NoLeafNames, NoSize>;
using NodeImplMap = NodeImpl<NoName, MultiLeaves, NoLeafNames, NoSize>;
using NodeImplUnion = NodeImpl<NoName, MultiLeaves, NoLeafNames, NoSize>;
using NodeImplFixed = NodeImpl<HasName, NoLeaves, NoLeafNames, HasSize>;

class AINGLE_DECL NodePrimitive : public NodeImplPrimitive {
public:
    explicit NodePrimitive(Type type) : NodeImplPrimitive(type) {}

    SchemaResolution resolve(const Node &reader) const override;

    void printJson(std::ostream &os, size_t depth) const override;

    bool isValid() const override {
        return true;
    }

    void printDefaultToJson(const GenericDatum &g, std::ostream &os, size_t depth) const override;
};

class AINGLE_DECL NodeSymbolic : public NodeImplSymbolic {
    using NodeWeakPtr = std::weak_ptr<Node>;

public:
    NodeSymbolic() : NodeImplSymbolic(AINGLE_SYMBOLIC) {}

    explicit NodeSymbolic(const HasName &name) : NodeImplSymbolic(AINGLE_SYMBOLIC, name, NoLeaves(), NoLeafNames(), NoSize()) {}

    NodeSymbolic(const HasName &name, const NodePtr &n) : NodeImplSymbolic(AINGLE_SYMBOLIC, name, NoLeaves(), NoLeafNames(), NoSize()), actualNode_(n) {}
    SchemaResolution resolve(const Node &reader) const override;

    void printJson(std::ostream &os, size_t depth) const override;

    bool isValid() const override {
        return (nameAttribute_.size() == 1);
    }

    void printDefaultToJson(const GenericDatum &g, std::ostream &os, size_t depth) const override;

    bool isSet() const {
        return (actualNode_.lock() != nullptr);
    }

    NodePtr getNode() const {
        NodePtr node = actualNode_.lock();
        if (!node) {
            throw Exception(boost::format("Could not follow symbol %1%") % name());
        }
        return node;
    }

    void setNode(const NodePtr &node) {
        actualNode_ = node;
    }

protected:
    NodeWeakPtr actualNode_;
};

class AINGLE_DECL NodeRecord : public NodeImplRecord {
    std::vector<GenericDatum> defaultValues;

public:
    NodeRecord() : NodeImplRecord(AINGLE_RECORD) {}
    NodeRecord(const HasName &name, const MultiLeaves &fields,
               const LeafNames &fieldsNames,
               std::vector<GenericDatum> dv);

    NodeRecord(const HasName &name, const HasDoc &doc, const MultiLeaves &fields,
               const LeafNames &fieldsNames,
               std::vector<GenericDatum> dv) : NodeImplRecord(AINGLE_RECORD, name, doc, fields, fieldsNames, NoSize()),
                                               defaultValues(std::move(dv)) {
        for (size_t i = 0; i < leafNameAttributes_.size(); ++i) {
            if (!nameIndex_.add(leafNameAttributes_.get(i), i)) {
                throw Exception(boost::format(
                                    "Cannot add duplicate field: %1%")
                                % leafNameAttributes_.get(i));
            }
        }
    }

    void swap(NodeRecord &r) {
        NodeImplRecord::swap(r);
        defaultValues.swap(r.defaultValues);
    }

    SchemaResolution resolve(const Node &reader) const override;

    void printJson(std::ostream &os, size_t depth) const override;

    bool isValid() const override {
        return ((nameAttribute_.size() == 1) && (leafAttributes_.size() == leafNameAttributes_.size()));
    }

    const GenericDatum &defaultValueAt(size_t index) override {
        return defaultValues[index];
    }

    void printDefaultToJson(const GenericDatum &g, std::ostream &os, size_t depth) const override;
};

class AINGLE_DECL NodeEnum : public NodeImplEnum {
public:
    NodeEnum() : NodeImplEnum(AINGLE_ENUM) {}

    NodeEnum(const HasName &name, const LeafNames &symbols) : NodeImplEnum(AINGLE_ENUM, name, NoLeaves(), symbols, NoSize()) {
        for (size_t i = 0; i < leafNameAttributes_.size(); ++i) {
            if (!nameIndex_.add(leafNameAttributes_.get(i), i)) {
                throw Exception(boost::format("Cannot add duplicate enum: %1%") % leafNameAttributes_.get(i));
            }
        }
    }

    SchemaResolution resolve(const Node &reader) const override;

    void printJson(std::ostream &os, size_t depth) const override;

    bool isValid() const override {
        return (
            (nameAttribute_.size() == 1) && (leafNameAttributes_.size() > 0));
    }

    void printDefaultToJson(const GenericDatum &g, std::ostream &os, size_t depth) const override;
};

class AINGLE_DECL NodeArray : public NodeImplArray {
public:
    NodeArray() : NodeImplArray(AINGLE_ARRAY) {}

    explicit NodeArray(const SingleLeaf &items) : NodeImplArray(AINGLE_ARRAY, NoName(), items, NoLeafNames(), NoSize()) {}

    SchemaResolution resolve(const Node &reader) const override;

    void printJson(std::ostream &os, size_t depth) const override;

    bool isValid() const override {
        return (leafAttributes_.size() == 1);
    }

    void printDefaultToJson(const GenericDatum &g, std::ostream &os, size_t depth) const override;
};

class AINGLE_DECL NodeMap : public NodeImplMap {
public:
    NodeMap();

    explicit NodeMap(const SingleLeaf &values) : NodeImplMap(AINGLE_MAP, NoName(), MultiLeaves(values), NoLeafNames(), NoSize()) {
        // need to add the key for the map too
        NodePtr key(new NodePrimitive(AINGLE_STRING));
        doAddLeaf(key);

        // key goes before value
        std::swap(leafAttributes_.get(0), leafAttributes_.get(1));
    }

    SchemaResolution resolve(const Node &reader) const override;

    void printJson(std::ostream &os, size_t depth) const override;

    bool isValid() const override {
        return (leafAttributes_.size() == 2);
    }

    void printDefaultToJson(const GenericDatum &g, std::ostream &os, size_t depth) const override;
};

class AINGLE_DECL NodeUnion : public NodeImplUnion {
public:
    NodeUnion() : NodeImplUnion(AINGLE_UNION) {}

    explicit NodeUnion(const MultiLeaves &types) : NodeImplUnion(AINGLE_UNION, NoName(), types, NoLeafNames(), NoSize()) {}

    SchemaResolution resolve(const Node &reader) const override;

    void printJson(std::ostream &os, size_t depth) const override;

    bool isValid() const override {
        std::set<std::string> seen;
        if (leafAttributes_.size() >= 1) {
            for (size_t i = 0; i < leafAttributes_.size(); ++i) {
                std::string name;
                const NodePtr &n = leafAttributes_.get(i);
                switch (n->type()) {
                    case AINGLE_STRING:
                        name = "string";
                        break;
                    case AINGLE_BYTES:
                        name = "bytes";
                        break;
                    case AINGLE_INT:
                        name = "int";
                        break;
                    case AINGLE_LONG:
                        name = "long";
                        break;
                    case AINGLE_FLOAT:
                        name = "float";
                        break;
                    case AINGLE_DOUBLE:
                        name = "double";
                        break;
                    case AINGLE_BOOL:
                        name = "bool";
                        break;
                    case AINGLE_NULL:
                        name = "null";
                        break;
                    case AINGLE_ARRAY:
                        name = "array";
                        break;
                    case AINGLE_MAP:
                        name = "map";
                        break;
                    case AINGLE_RECORD:
                    case AINGLE_ENUM:
                    case AINGLE_UNION:
                    case AINGLE_FIXED:
                    case AINGLE_SYMBOLIC:
                        name = n->name().fullname();
                        break;
                    default: return false;
                }
                if (seen.find(name) != seen.end()) {
                    return false;
                }
                seen.insert(name);
            }
            return true;
        }
        return false;
    }

    void printDefaultToJson(const GenericDatum &g, std::ostream &os, size_t depth) const override;
};

class AINGLE_DECL NodeFixed : public NodeImplFixed {
public:
    NodeFixed() : NodeImplFixed(AINGLE_FIXED) {}

    NodeFixed(const HasName &name, const HasSize &size) : NodeImplFixed(AINGLE_FIXED, name, NoLeaves(), NoLeafNames(), size) {}

    SchemaResolution resolve(const Node &reader) const override;

    void printJson(std::ostream &os, size_t depth) const override;

    bool isValid() const override {
        return (
            (nameAttribute_.size() == 1) && (sizeAttribute_.size() == 1));
    }

    void printDefaultToJson(const GenericDatum &g, std::ostream &os, size_t depth) const override;
};

template<class A, class B, class C, class D>
inline void
NodeImpl<A, B, C, D>::setLeafToSymbolic(size_t index, const NodePtr &node) {
    if (!B::hasAttribute) {
        throw Exception("Cannot change leaf node for nonexistent leaf");
    }

    auto &replaceNode = const_cast<NodePtr &>(leafAttributes_.get(index));
    if (replaceNode->name() != node->name()) {
        throw Exception("Symbolic name does not match the name of the schema it references");
    }

    auto symbol = std::make_shared<NodeSymbolic>();
    symbol->setName(node->name());
    symbol->setNode(node);
    replaceNode = symbol;
}

template<class A, class B, class C, class D>
inline void
NodeImpl<A, B, C, D>::printBasicInfo(std::ostream &os) const {
    os << type();
    if (hasName()) {
        os << ' ' << nameAttribute_.get();
    }

    if (D::hasAttribute) {
        os << " " << sizeAttribute_.get();
    }
    os << '\n';
    int count = leaves();
    count = count ? count : names();
    for (int i = 0; i < count; ++i) {
        if (C::hasAttribute) {
            os << "name " << nameAt(i) << '\n';
        }
        if (type() != AINGLE_SYMBOLIC && leafAttributes_.hasAttribute) {
            leafAt(i)->printBasicInfo(os);
        }
    }
    if (isCompound(type())) {
        os << "end " << type() << '\n';
    }
}

inline NodePtr resolveSymbol(const NodePtr &node) {
    if (node->type() != AINGLE_SYMBOLIC) {
        throw Exception("Only symbolic nodes may be resolved");
    }
    std::shared_ptr<NodeSymbolic> symNode = std::static_pointer_cast<NodeSymbolic>(node);
    return symNode->getNode();
}

template<typename T>
inline std::string intToHex(T i) {
    std::stringstream stream;
    stream << "\\u"
           << std::setfill('0') << std::setw(sizeof(T))
           << std::hex << i;
    return stream.str();
}

} // namespace aingle

#endif
