/**

 */

#include "GenericDatum.hh"
#include "NodeImpl.hh"

using std::string;
using std::vector;

namespace aingle {

GenericDatum::GenericDatum(const ValidSchema &schema) : type_(schema.root()->type()),
                                                        logicalType_(schema.root()->logicalType()) {
    init(schema.root());
}

GenericDatum::GenericDatum(const NodePtr &schema) : type_(schema->type()),
                                                    logicalType_(schema->logicalType()) {
    init(schema);
}

void GenericDatum::init(const NodePtr &schema) {
    NodePtr sc = schema;
    if (type_ == AINGLE_SYMBOLIC) {
        sc = resolveSymbol(schema);
        type_ = sc->type();
        logicalType_ = sc->logicalType();
    }
    switch (type_) {
        case AINGLE_NULL: break;
        case AINGLE_BOOL:
            value_ = bool();
            break;
        case AINGLE_INT:
            value_ = int32_t();
            break;
        case AINGLE_LONG:
            value_ = int64_t();
            break;
        case AINGLE_FLOAT:
            value_ = float();
            break;
        case AINGLE_DOUBLE:
            value_ = double();
            break;
        case AINGLE_STRING:
            value_ = string();
            break;
        case AINGLE_BYTES:
            value_ = vector<uint8_t>();
            break;
        case AINGLE_FIXED:
            value_ = GenericFixed(sc);
            break;
        case AINGLE_RECORD:
            value_ = GenericRecord(sc);
            break;
        case AINGLE_ENUM:
            value_ = GenericEnum(sc);
            break;
        case AINGLE_ARRAY:
            value_ = GenericArray(sc);
            break;
        case AINGLE_MAP:
            value_ = GenericMap(sc);
            break;
        case AINGLE_UNION:
            value_ = GenericUnion(sc);
            break;
        default:
            throw Exception(boost::format("Unknown schema type %1%") % toString(type_));
    }
}

GenericRecord::GenericRecord(const NodePtr &schema) : GenericContainer(AINGLE_RECORD, schema) {
    fields_.resize(schema->leaves());
    for (size_t i = 0; i < schema->leaves(); ++i) {
        fields_[i] = GenericDatum(schema->leafAt(i));
    }
}

GenericFixed::GenericFixed(const NodePtr &schema, const vector<uint8_t> &v) : GenericContainer(AINGLE_FIXED, schema), value_(v) {}
} // namespace aingle
