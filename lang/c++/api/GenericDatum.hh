/*

 */

#ifndef aingle_GenericDatum_hh__
#define aingle_GenericDatum_hh__

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#if __cplusplus >= 201703L
#include <any>
#else
#include "boost/any.hpp"
#endif

#include "LogicalType.hh"
#include "Node.hh"
#include "ValidSchema.hh"

namespace aingle {

/**
 * Generic datum which can hold any AIngle type. The datum has a type
 * and a value. The type is one of the AIngle data types. The C++ type for
 * value corresponds to the AIngle type.
 * \li An AIngle <tt>null</tt> corresponds to no C++ type. It is illegal to
 * to try to access values for <tt>null</tt>.
 * \li AIngle <tt>boolean</tt> maps to C++ <tt>bool</tt>
 * \li AIngle <tt>int</tt> maps to C++ <tt>int32_t</tt>.
 * \li AIngle <tt>long</tt> maps to C++ <tt>int64_t</tt>.
 * \li AIngle <tt>float</tt> maps to C++ <tt>float</tt>.
 * \li AIngle <tt>double</tt> maps to C++ <tt>double</tt>.
 * \li AIngle <tt>string</tt> maps to C++ <tt>std::string</tt>.
 * \li AIngle <tt>bytes</tt> maps to C++ <tt>std::vector&lt;uint_t&gt;</tt>.
 * \li AIngle <tt>fixed</tt> maps to C++ class <tt>GenericFixed</tt>.
 * \li AIngle <tt>enum</tt> maps to C++ class <tt>GenericEnum</tt>.
 * \li AIngle <tt>array</tt> maps to C++ class <tt>GenericArray</tt>.
 * \li AIngle <tt>map</tt> maps to C++ class <tt>GenericMap</tt>.
 * \li There is no C++ type corresponding to AIngle <tt>union</tt>. The
 * object should have the C++ type corresponding to one of the constituent
 * types of the union.
 *
 */
class AINGLE_DECL GenericDatum {
protected:
    Type type_;
    LogicalType logicalType_;
#if __cplusplus >= 201703L
    std::any value_;
#else
    boost::any value_;
#endif

    explicit GenericDatum(Type t)
        : type_(t), logicalType_(LogicalType::NONE) {}

    GenericDatum(Type t, LogicalType logicalType)
        : type_(t), logicalType_(logicalType) {}

    template<typename T>
    GenericDatum(Type t, LogicalType logicalType, const T &v)
        : type_(t), logicalType_(logicalType), value_(v) {}

    void init(const NodePtr &schema);

public:
    /**
     * The aingle data type this datum holds.
     */
    Type type() const;

    /**
     * The aingle logical type that augments the main data type this datum holds.
     */
    LogicalType logicalType() const;

    /**
     * Returns the value held by this datum.
     * T The type for the value. This must correspond to the
     * aingle type returned by type().
     */
    template<typename T>
    const T &value() const;

    /**
     * Returns the reference to the value held by this datum, which
     * can be used to change the contents. Please note that only
     * value can be changed, the data type of the value held cannot
     * be changed.
     *
     * T The type for the value. This must correspond to the
     * aingle type returned by type().
     */
    template<typename T>
    T &value();

    /**
     * Returns true if and only if this datum is a union.
     */
    bool isUnion() const { return type_ == AINGLE_UNION; }

    /**
     * Returns the index of the current branch, if this is a union.
     * \sa isUnion().
     */
    size_t unionBranch() const;

    /**
     * Selects a new branch in the union if this is a union.
     * \sa isUnion().
     */
    void selectBranch(size_t branch);

    /// Makes a new AINGLE_NULL datum.
    GenericDatum() : type_(AINGLE_NULL), logicalType_(LogicalType::NONE) {}

    /// Makes a new AINGLE_BOOL datum whose value is of type bool.
    /// We don't make this explicit constructor because we want to allow automatic conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    GenericDatum(bool v)
        : type_(AINGLE_BOOL), logicalType_(LogicalType::NONE), value_(v) {}

    /// Makes a new AINGLE_INT datum whose value is of type int32_t.
    /// We don't make this explicit constructor because we want to allow automatic conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    GenericDatum(int32_t v)
        : type_(AINGLE_INT), logicalType_(LogicalType::NONE), value_(v) {}

    /// Makes a new AINGLE_LONG datum whose value is of type int64_t.
    /// We don't make this explicit constructor because we want to allow automatic conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    GenericDatum(int64_t v)
        : type_(AINGLE_LONG), logicalType_(LogicalType::NONE), value_(v) {}

    /// Makes a new AINGLE_FLOAT datum whose value is of type float.
    /// We don't make this explicit constructor because we want to allow automatic conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    GenericDatum(float v)
        : type_(AINGLE_FLOAT), logicalType_(LogicalType::NONE), value_(v) {}

    /// Makes a new AINGLE_DOUBLE datum whose value is of type double.
    /// We don't make this explicit constructor because we want to allow automatic conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    GenericDatum(double v)
        : type_(AINGLE_DOUBLE), logicalType_(LogicalType::NONE), value_(v) {}

    /// Makes a new AINGLE_STRING datum whose value is of type std::string.
    /// We don't make this explicit constructor because we want to allow automatic conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    GenericDatum(const std::string &v)
        : type_(AINGLE_STRING), logicalType_(LogicalType::NONE), value_(v) {}

    /// Makes a new AINGLE_BYTES datum whose value is of type
    /// std::vector<uint8_t>.
    /// We don't make this explicit constructor because we want to allow automatic conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    GenericDatum(const std::vector<uint8_t> &v) : type_(AINGLE_BYTES), logicalType_(LogicalType::NONE), value_(v) {}

    /**
     * Constructs a datum corresponding to the given aingle type.
     * The value will the appropriate default corresponding to the
     * data type.
     * \param schema The schema that defines the aingle type.
     */
    /// We don't make this explicit constructor because we want to allow automatic conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    GenericDatum(const NodePtr &schema);

    /**
     * Constructs a datum corresponding to the given aingle type and set
     * the value.
     * \param schema The schema that defines the aingle type.
     * \param v The value for this type.
     */
    template<typename T>
    GenericDatum(const NodePtr &schema, const T &v) : type_(schema->type()), logicalType_(schema->logicalType()) {
        init(schema);
#if __cplusplus >= 201703L
        *std::any_cast<T>(&value_) = v;
#else
        *boost::any_cast<T>(&value_) = v;
#endif
    }

    /**
     * Constructs a datum corresponding to the given aingle type.
     * The value will the appropriate default corresponding to the
     * data type.
     * \param schema The schema that defines the aingle type.
     */
    explicit GenericDatum(const ValidSchema &schema);
};

/**
 * The base class for all generic type for containers.
 */
class AINGLE_DECL GenericContainer {
    NodePtr schema_;
    static void assertType(const NodePtr &schema, Type type);

protected:
    /**
     * Constructs a container corresponding to the given schema.
     */
    GenericContainer(Type type, const NodePtr &s) : schema_(s) {
        assertType(s, type);
    }

public:
    /// Returns the schema for this object
    const NodePtr &schema() const {
        return schema_;
    }
};

/**
 * Generic container for unions.
 */
class AINGLE_DECL GenericUnion : public GenericContainer {
    size_t curBranch_;
    GenericDatum datum_;

public:
    /**
     * Constructs a generic union corresponding to the given schema \p schema,
     * and the given value. The schema should be of AIngle type union
     * and the value should correspond to one of the branches of the union.
     */
    explicit GenericUnion(const NodePtr &schema) : GenericContainer(AINGLE_UNION, schema), curBranch_(schema->leaves()) {
        selectBranch(0);
    }

    /**
     * Returns the index of the current branch.
     */
    size_t currentBranch() const { return curBranch_; }

    /**
     * Selects a new branch. The type for the value is changed accordingly.
     * \param branch The index for the selected branch.
     */
    void selectBranch(size_t branch) {
        if (curBranch_ != branch) {
            datum_ = GenericDatum(schema()->leafAt(branch));
            curBranch_ = branch;
        }
    }

    /**
     * Returns the datum corresponding to the currently selected branch
     * in this union.
     */
    GenericDatum &datum() {
        return datum_;
    }

    /**
     * Returns the datum corresponding to the currently selected branch
     * in this union.
     */
    const GenericDatum &datum() const {
        return datum_;
    }
};

/**
 * The generic container for AIngle records.
 */
class AINGLE_DECL GenericRecord : public GenericContainer {
    std::vector<GenericDatum> fields_;

public:
    /**
     * Constructs a generic record corresponding to the given schema \p schema,
     * which should be of AIngle type record.
     */
    explicit GenericRecord(const NodePtr &schema);

    /**
     * Returns the number of fields in the current record.
     */
    size_t fieldCount() const {
        return fields_.size();
    }

    /**
     * Returns index of the field with the given name \p name
     */
    size_t fieldIndex(const std::string &name) const {
        size_t index = 0;
        if (!schema()->nameIndex(name, index)) {
            throw Exception("Invalid field name: " + name);
        }
        return index;
    }

    /**
     * Returns true if a field with the given name \p name is located in this r
     * false otherwise
     */
    bool hasField(const std::string &name) const {
        size_t index = 0;
        return schema()->nameIndex(name, index);
    }

    /**
     * Returns the field with the given name \p name.
     */
    const GenericDatum &field(const std::string &name) const {
        return fieldAt(fieldIndex(name));
    }

    /**
     * Returns the reference to the field with the given name \p name,
     * which can be used to change the contents.
     */
    GenericDatum &field(const std::string &name) {
        return fieldAt(fieldIndex(name));
    }

    /**
     * Returns the field at the given position \p pos.
     */
    const GenericDatum &fieldAt(size_t pos) const {
        return fields_[pos];
    }

    /**
     * Returns the reference to the field at the given position \p pos,
     * which can be used to change the contents.
     */
    GenericDatum &fieldAt(size_t pos) {
        return fields_[pos];
    }

    /**
     * Replaces the field at the given position \p pos with \p v.
     */
    void setFieldAt(size_t pos, const GenericDatum &v) {
        // assertSameType(v, schema()->leafAt(pos));
        fields_[pos] = v;
    }
};

/**
 * The generic container for AIngle arrays.
 */
class AINGLE_DECL GenericArray : public GenericContainer {
public:
    /**
     * The contents type for the array.
     */
    typedef std::vector<GenericDatum> Value;

    /**
     * Constructs a generic array corresponding to the given schema \p schema,
     * which should be of AIngle type array.
     */
    explicit GenericArray(const NodePtr &schema) : GenericContainer(AINGLE_ARRAY, schema) {
    }

    /**
     * Returns the contents of this array.
     */
    const Value &value() const {
        return value_;
    }

    /**
     * Returns the reference to the contents of this array.
     */
    Value &value() {
        return value_;
    }

private:
    Value value_;
};

/**
 * The generic container for AIngle maps.
 */
class AINGLE_DECL GenericMap : public GenericContainer {
public:
    /**
     * The contents type for the map.
     */
    typedef std::vector<std::pair<std::string, GenericDatum>> Value;

    /**
     * Constructs a generic map corresponding to the given schema \p schema,
     * which should be of AIngle type map.
     */
    explicit GenericMap(const NodePtr &schema) : GenericContainer(AINGLE_MAP, schema) {
    }

    /**
     * Returns the contents of this map.
     */
    const Value &value() const {
        return value_;
    }

    /**
     * Returns the reference to the contents of this map.
     */
    Value &value() {
        return value_;
    }

private:
    Value value_;
};

/**
 * Generic container for AIngle enum.
 */
class AINGLE_DECL GenericEnum : public GenericContainer {
    size_t value_;

    static size_t index(const NodePtr &schema, const std::string &symbol) {
        size_t result;
        if (schema->nameIndex(symbol, result)) {
            return result;
        }
        throw Exception("No such symbol");
    }

public:
    /**
     * Constructs a generic enum corresponding to the given schema \p schema,
     * which should be of AIngle type enum.
     */
    explicit GenericEnum(const NodePtr &schema) : GenericContainer(AINGLE_ENUM, schema), value_(0) {
    }

    GenericEnum(const NodePtr &schema, const std::string &symbol) : GenericContainer(AINGLE_ENUM, schema), value_(index(schema, symbol)) {
    }

    /**
     * Returns the symbol corresponding to the cardinal \p n. If the
     * value for \p n is not within the limits an exception is thrown.
     */
    const std::string &symbol(size_t n) {
        if (n < schema()->names()) {
            return schema()->nameAt(n);
        }
        throw Exception("Not as many symbols");
    }

    /**
     * Returns the cardinal for the given symbol \c symbol. If the symbol
     * is not defined for this enum and exception is thrown.
     */
    size_t index(const std::string &symbol) const {
        return index(schema(), symbol);
    }

    /**
     * Set the value for this enum corresponding to the given symbol \c symbol.
     */
    size_t set(const std::string &symbol) {
        return value_ = index(symbol);
    }

    /**
     * Set the value for this enum corresponding to the given cardinal \c n.
     */
    void set(size_t n) {
        if (n < schema()->names()) {
            value_ = n;
            return;
        }
        throw Exception("Not as many symbols");
    }

    /**
     * Returns the cardinal for the current value of this enum.
     */
    size_t value() const {
        return value_;
    }

    /**
     * Returns the symbol for the current value of this enum.
     */
    const std::string &symbol() const {
        return schema()->nameAt(value_);
    }
};

/**
 * Generic container for AIngle fixed.
 */
class AINGLE_DECL GenericFixed : public GenericContainer {
    std::vector<uint8_t> value_;

public:
    /**
     * Constructs a generic enum corresponding to the given schema \p schema,
     * which should be of AIngle type fixed.
     */
    explicit GenericFixed(const NodePtr &schema) : GenericContainer(AINGLE_FIXED, schema) {
        value_.resize(schema->fixedSize());
    }

    GenericFixed(const NodePtr &schema, const std::vector<uint8_t> &v);

    /**
     * Returns the contents of this fixed.
     */
    const std::vector<uint8_t> &value() const {
        return value_;
    }

    /**
     * Returns the reference to the contents of this fixed.
     */
    std::vector<uint8_t> &value() {
        return value_;
    }
};

inline Type GenericDatum::type() const {
    return (type_ == AINGLE_UNION) ?
#if __cplusplus >= 201703L
                                 std::any_cast<GenericUnion>(&value_)->datum().type()
                                 :
#else
                                 boost::any_cast<GenericUnion>(&value_)->datum().type()
                                 :
#endif
                                 type_;
}

inline LogicalType GenericDatum::logicalType() const {
    return logicalType_;
}

template<typename T>
T &GenericDatum::value() {
    return (type_ == AINGLE_UNION) ?
#if __cplusplus >= 201703L
                                 std::any_cast<GenericUnion>(&value_)->datum().value<T>()
                                 : *std::any_cast<T>(&value_);
#else
                                 boost::any_cast<GenericUnion>(&value_)->datum().value<T>()
                                 : *boost::any_cast<T>(&value_);
#endif
}

template<typename T>
const T &GenericDatum::value() const {
    return (type_ == AINGLE_UNION) ?
#if __cplusplus >= 201703L
                                 std::any_cast<GenericUnion>(&value_)->datum().value<T>()
                                 : *std::any_cast<T>(&value_);
#else
                                 boost::any_cast<GenericUnion>(&value_)->datum().value<T>()
                                 : *boost::any_cast<T>(&value_);
#endif
}

inline size_t GenericDatum::unionBranch() const {
#if __cplusplus >= 201703L
    return std::any_cast<GenericUnion>(&value_)->currentBranch();
#else
    return boost::any_cast<GenericUnion>(&value_)->currentBranch();
#endif
}

inline void GenericDatum::selectBranch(size_t branch) {
#if __cplusplus >= 201703L
    std::any_cast<GenericUnion>(&value_)->selectBranch(branch);
#else
    boost::any_cast<GenericUnion>(&value_)->selectBranch(branch);
#endif
}

} // namespace aingle
#endif // aingle_GenericDatum_hh__
