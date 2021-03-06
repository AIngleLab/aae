/*

 */

#ifndef aingle_Encoder_hh__
#define aingle_Encoder_hh__

#include "Config.hh"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Stream.hh"
#include "ValidSchema.hh"

/// \file
///
/// Low level support for encoding aingle values.
/// This class has two types of functions.  One type of functions support
/// the writing of leaf values (for example, encodeLong and
/// encodeString).  These functions have analogs in Decoder.
///
/// The other type of functions support the writing of maps and arrays.
/// These functions are arrayStart, startItem, and arrayEnd
/// (and similar functions for maps).
/// Some implementations of Encoder handle the
/// buffering required to break large maps and arrays into blocks,
/// which is necessary for applications that want to do streaming.

namespace aingle {

/**
 * The abstract base class for all AIngle encoders. The implementations
 * differ in the method of encoding (binary versus JSON) or in capabilities
 * such as ability to verify the order of invocation of different functions.
 */
class AINGLE_DECL Encoder {
public:
    virtual ~Encoder() = default;
    /// All future encodings will go to os, which should be valid until
    /// it is reset with another call to init() or the encoder is
    /// destructed.
    virtual void init(OutputStream &os) = 0;

    /// Flushes any data in internal buffers.
    virtual void flush() = 0;

    /// Returns the number of bytes produced so far.
    /// For a meaningful value, do a flush() before invoking this function.
    virtual int64_t byteCount() const = 0;

    /// Encodes a null to the current stream.
    virtual void encodeNull() = 0;

    /// Encodes a bool to the current stream
    virtual void encodeBool(bool b) = 0;

    /// Encodes a 32-bit int to the current stream.
    virtual void encodeInt(int32_t i) = 0;

    /// Encodes a 64-bit signed int to the current stream.
    virtual void encodeLong(int64_t l) = 0;

    /// Encodes a single-precision floating point number to the current stream.
    virtual void encodeFloat(float f) = 0;

    /// Encodes a double-precision floating point number to the current stream.
    virtual void encodeDouble(double d) = 0;

    /// Encodes a UTF-8 string to the current stream.
    virtual void encodeString(const std::string &s) = 0;

    /**
     * Encodes arbitrary binary data into the current stream as AIngle "bytes"
     * data type.
     * \param bytes Where the data is
     * \param len Number of bytes at \p bytes.
     */
    virtual void encodeBytes(const uint8_t *bytes, size_t len) = 0;

    /**
     * Encodes arbitrary binary data into the current stream as AIngle "bytes"
     * data type.
     * \param bytes The data.
     */
    void encodeBytes(const std::vector<uint8_t> &bytes) {
        uint8_t b = 0;
        encodeBytes(bytes.empty() ? &b : bytes.data(), bytes.size());
    }

    /// Encodes fixed length binary to the current stream.
    virtual void encodeFixed(const uint8_t *bytes, size_t len) = 0;

    /**
     * Encodes an AIngle data type Fixed.
     * \param bytes The fixed, the length of which is taken as the size
     * of fixed.
     */
    void encodeFixed(const std::vector<uint8_t> &bytes) {
        encodeFixed(bytes.data(), bytes.size());
    }

    /// Encodes enum to the current stream.
    virtual void encodeEnum(size_t e) = 0;

    /// Indicates that an array of items is being encoded.
    virtual void arrayStart() = 0;

    /// Indicates that the current array of items have ended.
    virtual void arrayEnd() = 0;

    /// Indicates that a map of items is being encoded.
    virtual void mapStart() = 0;

    /// Indicates that the current map of items have ended.
    virtual void mapEnd() = 0;

    /// Indicates that count number of items are to follow in the current array
    /// or map.
    virtual void setItemCount(size_t count) = 0;

    /// Marks a beginning of an item in the current array or map.
    virtual void startItem() = 0;

    /// Encodes a branch of a union. The actual value is to follow.
    virtual void encodeUnionIndex(size_t e) = 0;
};

/**
 * Shared pointer to Encoder.
 */
using EncoderPtr = std::shared_ptr<Encoder>;

/**
 *  Returns an encoder that can encode binary AIngle standard.
 */
AINGLE_DECL EncoderPtr binaryEncoder();

/**
 *  Returns an encoder that validates sequence of calls to an underlying
 *  Encoder against the given schema.
 */
AINGLE_DECL EncoderPtr validatingEncoder(const ValidSchema &schema,
                                       const EncoderPtr &base);

/**
 *  Returns an encoder that encodes AIngle standard for JSON.
 */
AINGLE_DECL EncoderPtr jsonEncoder(const ValidSchema &schema);

/**
 *  Returns an encoder that encodes AIngle standard for pretty printed JSON.
 */
AINGLE_DECL EncoderPtr jsonPrettyEncoder(const ValidSchema &schema);

} // namespace aingle

#endif
