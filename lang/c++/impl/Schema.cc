/**

 */

#include <utility>

#include "Schema.hh"

namespace aingle {

RecordSchema::RecordSchema(const std::string &name) : Schema(new NodeRecord) {
    node_->setName(Name(name));
}

void RecordSchema::addField(const std::string &name, const Schema &fieldSchema) {
    // add the name first. it will throw if the name is a duplicate, preventing
    // the leaf from being added
    node_->addName(name);

    node_->addLeaf(fieldSchema.root());
}

std::string RecordSchema::getDoc() const {
    return node_->getDoc();
}
void RecordSchema::setDoc(const std::string &doc) {
    node_->setDoc(doc);
}

EnumSchema::EnumSchema(const std::string &name) : Schema(new NodeEnum) {
    node_->setName(Name(name));
}

void EnumSchema::addSymbol(const std::string &symbol) {
    node_->addName(symbol);
}

ArraySchema::ArraySchema(const Schema &itemsSchema) : Schema(new NodeArray) {
    node_->addLeaf(itemsSchema.root());
}

ArraySchema::ArraySchema(const ArraySchema &itemsSchema) : Schema(new NodeArray) {
    node_->addLeaf(itemsSchema.root());
}

MapSchema::MapSchema(const Schema &valuesSchema) : Schema(new NodeMap) {
    node_->addLeaf(valuesSchema.root());
}

MapSchema::MapSchema(const MapSchema &valuesSchema) : Schema(new NodeMap) {
    node_->addLeaf(valuesSchema.root());
}

UnionSchema::UnionSchema() : Schema(new NodeUnion) {}

void UnionSchema::addType(const Schema &typeSchema) {
    if (typeSchema.type() == AINGLE_UNION) {
        throw Exception("Cannot add unions to unions");
    }

    if (typeSchema.type() == AINGLE_RECORD) {
        // check for duplicate records
        size_t types = node_->leaves();
        for (size_t i = 0; i < types; ++i) {
            const NodePtr &leaf = node_->leafAt(i);
            // TODO, more checks?
            if (leaf->type() == AINGLE_RECORD && leaf->name() == typeSchema.root()->name()) {
                throw Exception("Records in unions cannot have duplicate names");
            }
        }
    }

    node_->addLeaf(typeSchema.root());
}

FixedSchema::FixedSchema(int size, const std::string &name) : Schema(new NodeFixed) {
    node_->setFixedSize(size);
    node_->setName(Name(name));
}

SymbolicSchema::SymbolicSchema(const Name &name, const NodePtr &link) : Schema(new NodeSymbolic(HasName(name), link)) {
}

} // namespace aingle
