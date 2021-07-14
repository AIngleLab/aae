/**

 */

#include "Generic.hh"
#include <utility>

namespace aingle {

using std::ostringstream;
using std::string;
using std::vector;

typedef vector<uint8_t> bytes;

void GenericContainer::assertType(const NodePtr &schema, Type type) {
    if (schema->type() != type) {
        throw Exception(boost::format("Schema type %1 expected %2") % toString(schema->type()) % toString(type));
    }
}

GenericReader::GenericReader(ValidSchema s, const DecoderPtr &decoder) : schema_(std::move(s)), isResolving_(dynamic_cast<ResolvingDecoder *>(&(*decoder)) != nullptr),
                                                                         decoder_(decoder) {
}

GenericReader::GenericReader(const ValidSchema &writerSchema,
                             const ValidSchema &readerSchema, const DecoderPtr &decoder) : schema_(readerSchema),
                                                                                           isResolving_(true),
                                                                                           decoder_(resolvingDecoder(writerSchema, readerSchema, decoder)) {
}

void GenericReader::read(GenericDatum &datum) const {
    datum = GenericDatum(schema_.root());
    read(datum, *decoder_, isResolving_);
}

void GenericReader::read(GenericDatum &datum, Decoder &d, bool isResolving) {
    if (datum.isUnion()) {
        datum.selectBranch(d.decodeUnionIndex());
    }
    switch (datum.type()) {
        case AINGLE_NULL:
            d.decodeNull();
            break;
        case AINGLE_BOOL:
            datum.value<bool>() = d.decodeBool();
            break;
        case AINGLE_INT:
            datum.value<int32_t>() = d.decodeInt();
            break;
        case AINGLE_LONG:
            datum.value<int64_t>() = d.decodeLong();
            break;
        case AINGLE_FLOAT:
            datum.value<float>() = d.decodeFloat();
            break;
        case AINGLE_DOUBLE:
            datum.value<double>() = d.decodeDouble();
            break;
        case AINGLE_STRING:
            d.decodeString(datum.value<string>());
            break;
        case AINGLE_BYTES:
            d.decodeBytes(datum.value<bytes>());
            break;
        case AINGLE_FIXED: {
            auto &f = datum.value<GenericFixed>();
            d.decodeFixed(f.schema()->fixedSize(), f.value());
        } break;
        case AINGLE_RECORD: {
            auto &r = datum.value<GenericRecord>();
            size_t c = r.schema()->leaves();
            if (isResolving) {
                std::vector<size_t> fo =
                    static_cast<ResolvingDecoder &>(d).fieldOrder();
                for (size_t i = 0; i < c; ++i) {
                    read(r.fieldAt(fo[i]), d, isResolving);
                }
            } else {
                for (size_t i = 0; i < c; ++i) {
                    read(r.fieldAt(i), d, isResolving);
                }
            }
        } break;
        case AINGLE_ENUM:
            datum.value<GenericEnum>().set(d.decodeEnum());
            break;
        case AINGLE_ARRAY: {
            auto &v = datum.value<GenericArray>();
            vector<GenericDatum> &r = v.value();
            const NodePtr &nn = v.schema()->leafAt(0);
            r.resize(0);
            size_t start = 0;
            for (size_t m = d.arrayStart(); m != 0; m = d.arrayNext()) {
                r.resize(r.size() + m);
                for (; start < r.size(); ++start) {
                    r[start] = GenericDatum(nn);
                    read(r[start], d, isResolving);
                }
            }
        } break;
        case AINGLE_MAP: {
            auto &v = datum.value<GenericMap>();
            GenericMap::Value &r = v.value();
            const NodePtr &nn = v.schema()->leafAt(1);
            r.resize(0);
            size_t start = 0;
            for (size_t m = d.mapStart(); m != 0; m = d.mapNext()) {
                r.resize(r.size() + m);
                for (; start < r.size(); ++start) {
                    d.decodeString(r[start].first);
                    r[start].second = GenericDatum(nn);
                    read(r[start].second, d, isResolving);
                }
            }
        } break;
        default:
            throw Exception(boost::format("Unknown schema type %1%") % toString(datum.type()));
    }
}

void GenericReader::read(Decoder &d, GenericDatum &g, const ValidSchema &s) {
    g = GenericDatum(s);
    read(d, g);
}

void GenericReader::read(Decoder &d, GenericDatum &g) {
    read(g, d, dynamic_cast<ResolvingDecoder *>(&d) != nullptr);
}

GenericWriter::GenericWriter(ValidSchema s, EncoderPtr encoder) : schema_(std::move(s)), encoder_(std::move(encoder)) {
}

void GenericWriter::write(const GenericDatum &datum) const {
    write(datum, *encoder_);
}

void GenericWriter::write(const GenericDatum &datum, Encoder &e) {
    if (datum.isUnion()) {
        e.encodeUnionIndex(datum.unionBranch());
    }
    switch (datum.type()) {
        case AINGLE_NULL:
            e.encodeNull();
            break;
        case AINGLE_BOOL:
            e.encodeBool(datum.value<bool>());
            break;
        case AINGLE_INT:
            e.encodeInt(datum.value<int32_t>());
            break;
        case AINGLE_LONG:
            e.encodeLong(datum.value<int64_t>());
            break;
        case AINGLE_FLOAT:
            e.encodeFloat(datum.value<float>());
            break;
        case AINGLE_DOUBLE:
            e.encodeDouble(datum.value<double>());
            break;
        case AINGLE_STRING:
            e.encodeString(datum.value<string>());
            break;
        case AINGLE_BYTES:
            e.encodeBytes(datum.value<bytes>());
            break;
        case AINGLE_FIXED:
            e.encodeFixed(datum.value<GenericFixed>().value());
            break;
        case AINGLE_RECORD: {
            const auto &r = datum.value<GenericRecord>();
            size_t c = r.schema()->leaves();
            for (size_t i = 0; i < c; ++i) {
                write(r.fieldAt(i), e);
            }
        } break;
        case AINGLE_ENUM:
            e.encodeEnum(datum.value<GenericEnum>().value());
            break;
        case AINGLE_ARRAY: {
            const GenericArray::Value &r = datum.value<GenericArray>().value();
            e.arrayStart();
            if (!r.empty()) {
                e.setItemCount(r.size());
                for (const auto &it : r) {
                    e.startItem();
                    write(it, e);
                }
            }
            e.arrayEnd();
        } break;
        case AINGLE_MAP: {
            const GenericMap::Value &r = datum.value<GenericMap>().value();
            e.mapStart();
            if (!r.empty()) {
                e.setItemCount(r.size());
                for (const auto &it : r) {
                    e.startItem();
                    e.encodeString(it.first);
                    write(it.second, e);
                }
            }
            e.mapEnd();
        } break;
        default:
            throw Exception(boost::format("Unknown schema type %1%") % toString(datum.type()));
    }
}

void GenericWriter::write(Encoder &e, const GenericDatum &g) {
    write(g, e);
}

} // namespace aingle
