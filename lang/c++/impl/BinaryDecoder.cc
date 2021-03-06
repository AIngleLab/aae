/**

 */

#include "Decoder.hh"
#include "Exception.hh"
#include "Zigzag.hh"
#include <memory>

namespace aingle {

using std::make_shared;

class BinaryDecoder : public Decoder {
    StreamReader in_;

    void init(InputStream &is) final;
    void decodeNull() final;
    bool decodeBool() final;
    int32_t decodeInt() final;
    int64_t decodeLong() final;
    float decodeFloat() final;
    double decodeDouble() final;
    void decodeString(std::string &value) final;
    void skipString() final;
    void decodeBytes(std::vector<uint8_t> &value) final;
    void skipBytes() final;
    void decodeFixed(size_t n, std::vector<uint8_t> &value) final;
    void skipFixed(size_t n) final;
    size_t decodeEnum() final;
    size_t arrayStart() final;
    size_t arrayNext() final;
    size_t skipArray() final;
    size_t mapStart() final;
    size_t mapNext() final;
    size_t skipMap() final;
    size_t decodeUnionIndex() final;

    int64_t doDecodeLong();
    size_t doDecodeItemCount();
    size_t doDecodeLength();
    void drain() final;
};

DecoderPtr binaryDecoder() {
    return make_shared<BinaryDecoder>();
}

void BinaryDecoder::init(InputStream &is) {
    in_.reset(is);
}

void BinaryDecoder::decodeNull() {
}

bool BinaryDecoder::decodeBool() {
    auto v = in_.read();
    if (v == 0) {
        return false;
    } else if (v == 1) {
        return true;
    }
    throw Exception(boost::format("Invalid value for bool: %1%") % v);
}

int32_t BinaryDecoder::decodeInt() {
    auto val = doDecodeLong();
    if (val < INT32_MIN || val > INT32_MAX) {
        throw Exception(
            boost::format("Value out of range for AIngle int: %1%") % val);
    }
    return static_cast<int32_t>(val);
}

int64_t BinaryDecoder::decodeLong() {
    return doDecodeLong();
}

float BinaryDecoder::decodeFloat() {
    float result;
    in_.readBytes(reinterpret_cast<uint8_t *>(&result), sizeof(float));
    return result;
}

double BinaryDecoder::decodeDouble() {
    double result;
    in_.readBytes(reinterpret_cast<uint8_t *>(&result), sizeof(double));
    return result;
}

size_t BinaryDecoder::doDecodeLength() {
    ssize_t len = decodeInt();
    if (len < 0) {
        throw Exception(
            boost::format("Cannot have negative length: %1%") % len);
    }
    return len;
}

void BinaryDecoder::drain() {
    in_.drain(false);
}

void BinaryDecoder::decodeString(std::string &value) {
    size_t len = doDecodeLength();
    value.resize(len);
    if (len > 0) {
        in_.readBytes(const_cast<uint8_t *>(
                          reinterpret_cast<const uint8_t *>(value.c_str())),
                      len);
    }
}

void BinaryDecoder::skipString() {
    size_t len = doDecodeLength();
    in_.skipBytes(len);
}

void BinaryDecoder::decodeBytes(std::vector<uint8_t> &value) {
    size_t len = doDecodeLength();
    value.resize(len);
    if (len > 0) {
        in_.readBytes(value.data(), len);
    }
}

void BinaryDecoder::skipBytes() {
    size_t len = doDecodeLength();
    in_.skipBytes(len);
}

void BinaryDecoder::decodeFixed(size_t n, std::vector<uint8_t> &value) {
    value.resize(n);
    if (n > 0) {
        in_.readBytes(value.data(), n);
    }
}

void BinaryDecoder::skipFixed(size_t n) {
    in_.skipBytes(n);
}

size_t BinaryDecoder::decodeEnum() {
    return static_cast<size_t>(doDecodeLong());
}

size_t BinaryDecoder::arrayStart() {
    return doDecodeItemCount();
}

size_t BinaryDecoder::doDecodeItemCount() {
    auto result = doDecodeLong();
    if (result < 0) {
        doDecodeLong();
        return static_cast<size_t>(-result);
    }
    return static_cast<size_t>(result);
}

size_t BinaryDecoder::arrayNext() {
    return static_cast<size_t>(doDecodeLong());
}

size_t BinaryDecoder::skipArray() {
    for (;;) {
        auto r = doDecodeLong();
        if (r < 0) {
            auto n = static_cast<size_t>(doDecodeLong());
            in_.skipBytes(n);
        } else {
            return static_cast<size_t>(r);
        }
    }
}

size_t BinaryDecoder::mapStart() {
    return doDecodeItemCount();
}

size_t BinaryDecoder::mapNext() {
    return doDecodeItemCount();
}

size_t BinaryDecoder::skipMap() {
    return skipArray();
}

size_t BinaryDecoder::decodeUnionIndex() {
    return static_cast<size_t>(doDecodeLong());
}

int64_t BinaryDecoder::doDecodeLong() {
    uint64_t encoded = 0;
    int shift = 0;
    uint8_t u;
    do {
        if (shift >= 64) {
            throw Exception("Invalid AIngle varint");
        }
        u = in_.read();
        encoded |= static_cast<uint64_t>(u & 0x7f) << shift;
        shift += 7;
    } while (u & 0x80);

    return decodeZigzag64(encoded);
}

} // namespace aingle
