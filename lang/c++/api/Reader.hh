/*

 */

#ifndef aingle_Reader_hh__
#define aingle_Reader_hh__

#include <array>
#include <boost/noncopyable.hpp>
#include <cstdint>
#include <vector>

#include "Config.hh"
#include "Types.hh"
#include "Validator.hh"
#include "Zigzag.hh"
#include "buffer/BufferReader.hh"

namespace aingle {

///
/// Parses from an aingle encoding to the requested type.  Assumes the next item
/// in the aingle binary data is the expected type.
///

template<class ValidatorType>
class ReaderImpl : private boost::noncopyable {

public:
    explicit ReaderImpl(const InputBuffer &buffer) : reader_(buffer) {}

    ReaderImpl(const ValidSchema &schema, const InputBuffer &buffer) : validator_(schema),
                                                                       reader_(buffer) {}

    void readValue(Null &) {
        validator_.checkTypeExpected(AINGLE_NULL);
    }

    void readValue(bool &val) {
        validator_.checkTypeExpected(AINGLE_BOOL);
        uint8_t intVal = 0;
        reader_.read(intVal);
        val = (intVal != 0);
    }

    void readValue(int32_t &val) {
        validator_.checkTypeExpected(AINGLE_INT);
        auto encoded = static_cast<uint32_t>(readVarInt());
        val = decodeZigzag32(encoded);
    }

    void readValue(int64_t &val) {
        validator_.checkTypeExpected(AINGLE_LONG);
        uint64_t encoded = readVarInt();
        val = decodeZigzag64(encoded);
    }

    void readValue(float &val) {
        validator_.checkTypeExpected(AINGLE_FLOAT);
        union {
            float f;
            uint32_t i;
        } v;
        reader_.read(v.i);
        val = v.f;
    }

    void readValue(double &val) {
        validator_.checkTypeExpected(AINGLE_DOUBLE);
        union {
            double d;
            uint64_t i;
        } v;
        reader_.read(v.i);
        val = v.d;
    }

    void readValue(std::string &val) {
        validator_.checkTypeExpected(AINGLE_STRING);
        auto size = static_cast<size_t>(readSize());
        reader_.read(val, size);
    }

    void readBytes(std::vector<uint8_t> &val) {
        validator_.checkTypeExpected(AINGLE_BYTES);
        auto size = static_cast<size_t>(readSize());
        val.resize(size);
        reader_.read(reinterpret_cast<char *>(val.data()), size);
    }

    void readFixed(uint8_t *val, size_t size) {
        validator_.checkFixedSizeExpected(size);
        reader_.read(reinterpret_cast<char *>(val), size);
    }

    template<size_t N>
    void readFixed(uint8_t (&val)[N]) {
        this->readFixed(val, N);
    }

    template<size_t N>
    void readFixed(std::array<uint8_t, N> &val) {
        this->readFixed(val.data(), N);
    }

    void readRecord() {
        validator_.checkTypeExpected(AINGLE_RECORD);
        validator_.checkTypeExpected(AINGLE_LONG);
        validator_.setCount(1);
    }

    void readRecordEnd() {
        validator_.checkTypeExpected(AINGLE_RECORD);
        validator_.checkTypeExpected(AINGLE_LONG);
        validator_.setCount(0);
    }

    int64_t readArrayBlockSize() {
        validator_.checkTypeExpected(AINGLE_ARRAY);
        return readCount();
    }

    int64_t readUnion() {
        validator_.checkTypeExpected(AINGLE_UNION);
        return readCount();
    }

    int64_t readEnum() {
        validator_.checkTypeExpected(AINGLE_ENUM);
        return readCount();
    }

    int64_t readMapBlockSize() {
        validator_.checkTypeExpected(AINGLE_MAP);
        return readCount();
    }

    Type nextType() const {
        return validator_.nextTypeExpected();
    }

    bool currentRecordName(std::string &name) const {
        return validator_.getCurrentRecordName(name);
    }

    bool nextFieldName(std::string &name) const {
        return validator_.getNextFieldName(name);
    }

private:
    uint64_t readVarInt() {
        uint64_t encoded = 0;
        uint8_t val = 0;
        int shift = 0;
        do {
            reader_.read(val);
            uint64_t newBits = static_cast<uint64_t>(val & 0x7f) << shift;
            encoded |= newBits;
            shift += 7;
        } while (val & 0x80);

        return encoded;
    }

    int64_t readSize() {
        uint64_t encoded = readVarInt();
        int64_t size = decodeZigzag64(encoded);
        return size;
    }

    int64_t readCount() {
        validator_.checkTypeExpected(AINGLE_LONG);
        int64_t count = readSize();
        validator_.setCount(count);
        return count;
    }

    ValidatorType validator_;
    BufferReader reader_;
};

using Reader = ReaderImpl<NullValidator>;
using ValidatingReader = ReaderImpl<Validator>;

} // namespace aingle

#endif
