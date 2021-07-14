/*

 */

#ifndef aingle_Serializer_hh__
#define aingle_Serializer_hh__

#include <array>
#include <boost/noncopyable.hpp>

#include "Config.hh"
#include "Writer.hh"

namespace aingle {

/// Class that wraps a Writer or ValidatingWriter with an interface that uses
/// explicit write* names instead of writeValue

template<class Writer>
class Serializer : private boost::noncopyable {

public:
    /// Constructor only works with Writer
    explicit Serializer() : writer_() {}

    /// Constructor only works with ValidatingWriter
    explicit Serializer(const ValidSchema &schema) : writer_(schema) {}

    void writeNull() {
        writer_.writeValue(Null());
    }

    void writeBool(bool val) {
        writer_.writeValue(val);
    }

    void writeInt(int32_t val) {
        writer_.writeValue(val);
    }

    void writeLong(int64_t val) {
        writer_.writeValue(val);
    }

    void writeFloat(float val) {
        writer_.writeValue(val);
    }

    void writeDouble(double val) {
        writer_.writeValue(val);
    }

    void writeBytes(const void *val, size_t size) {
        writer_.writeBytes(val, size);
    }

    template<size_t N>
    void writeFixed(const uint8_t (&val)[N]) {
        writer_.writeFixed(val);
    }

    template<size_t N>
    void writeFixed(const std::array<uint8_t, N> &val) {
        writer_.writeFixed(val);
    }

    void writeString(const std::string &val) {
        writer_.writeValue(val);
    }

    void writeRecord() {
        writer_.writeRecord();
    }

    void writeRecordEnd() {
        writer_.writeRecordEnd();
    }

    void writeArrayBlock(int64_t size) {
        writer_.writeArrayBlock(size);
    }

    void writeArrayEnd() {
        writer_.writeArrayEnd();
    }

    void writeMapBlock(int64_t size) {
        writer_.writeMapBlock(size);
    }

    void writeMapEnd() {
        writer_.writeMapEnd();
    }

    void writeUnion(int64_t choice) {
        writer_.writeUnion(choice);
    }

    void writeEnum(int64_t choice) {
        writer_.writeEnum(choice);
    }

    InputBuffer buffer() const {
        return writer_.buffer();
    }

private:
    Writer writer_;
};

} // namespace aingle

#endif
