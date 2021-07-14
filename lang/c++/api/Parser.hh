/*

 */

#ifndef aingle_Parser_hh__
#define aingle_Parser_hh__

#include "Config.hh"
#include "Reader.hh"

#include <array>

namespace aingle {

///
/// Class that wraps a reader or ValidatingReade with an interface that uses
/// explicit get* names instead of getValue
///

template<class Reader>
class Parser : private boost::noncopyable {

public:
    // Constructor only works with Writer
    explicit Parser(const InputBuffer &in) : reader_(in) {}

    /// Constructor only works with ValidatingWriter
    Parser(const ValidSchema &schema, const InputBuffer &in) : reader_(schema, in) {}

    void readNull() {
        Null null;
        reader_.readValue(null);
    }

    bool readBool() {
        bool val;
        reader_.readValue(val);
        return val;
    }

    int32_t readInt() {
        int32_t val;
        reader_.readValue(val);
        return val;
    }

    int64_t readLong() {
        int64_t val;
        reader_.readValue(val);
        return val;
    }

    float readFloat() {
        float val;
        reader_.readValue(val);
        return val;
    }

    double readDouble() {
        double val;
        reader_.readValue(val);
        return val;
    }

    void readString(std::string &val) {
        reader_.readValue(val);
    }

    void readBytes(std::vector<uint8_t> &val) {
        reader_.readBytes(val);
    }

    template<size_t N>
    void readFixed(uint8_t (&val)[N]) {
        reader_.readFixed(val);
    }

    template<size_t N>
    void readFixed(std::array<uint8_t, N> &val) {
        reader_.readFixed(val);
    }

    void readRecord() {
        reader_.readRecord();
    }

    void readRecordEnd() {
        reader_.readRecordEnd();
    }

    int64_t readArrayBlockSize() {
        return reader_.readArrayBlockSize();
    }

    int64_t readUnion() {
        return reader_.readUnion();
    }

    int64_t readEnum() {
        return reader_.readEnum();
    }

    int64_t readMapBlockSize() {
        return reader_.readMapBlockSize();
    }

private:
    friend Type nextType(Parser<ValidatingReader> &p);
    friend bool currentRecordName(Parser<ValidatingReader> &p, std::string &name);
    friend bool nextFieldName(Parser<ValidatingReader> &p, std::string &name);

    Reader reader_;
};

inline Type nextType(Parser<ValidatingReader> &p) {
    return p.reader_.nextType();
}

inline bool currentRecordName(Parser<ValidatingReader> &p, std::string &name) {
    return p.reader_.currentRecordName(name);
}

inline bool nextFieldName(Parser<ValidatingReader> &p, std::string &name) {
    return p.reader_.nextFieldName(name);
}

} // namespace aingle

#endif
