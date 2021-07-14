/**

 */

#ifndef aingle_json_JsonDom_hh__
#define aingle_json_JsonDom_hh__

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Config.hh"
#include "boost/any.hpp"

namespace aingle {

class AINGLE_DECL InputStream;

namespace json {
class Entity;

typedef bool Bool;
typedef int64_t Long;
typedef double Double;
typedef std::string String;
typedef std::vector<Entity> Array;
typedef std::map<std::string, Entity> Object;

class AINGLE_DECL JsonParser;
class JsonNullFormatter;

template<typename F = JsonNullFormatter>
class AINGLE_DECL JsonGenerator;

enum class EntityType {
    Null,
    Bool,
    Long,
    Double,
    String,
    Arr,
    Obj
};

const char *typeToString(EntityType t);

inline std::ostream &operator<<(std::ostream &os, EntityType et) {
    return os << typeToString(et);
}

class AINGLE_DECL Entity {
    EntityType type_;
    boost::any value_;
    size_t line_; // can't be const else noncopyable...

    void ensureType(EntityType) const;

public:
    explicit Entity(size_t line = 0) : type_(EntityType::Null), line_(line) {}
    // Not explicit because do want implicit conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    Entity(Bool v, size_t line = 0) : type_(EntityType::Bool), value_(v), line_(line) {}
    // Not explicit because do want implicit conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    Entity(Long v, size_t line = 0) : type_(EntityType::Long), value_(v), line_(line) {}
    // Not explicit because do want implicit conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    Entity(Double v, size_t line = 0) : type_(EntityType::Double), value_(v), line_(line) {}
    // Not explicit because do want implicit conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    Entity(const std::shared_ptr<String> &v, size_t line = 0) : type_(EntityType::String), value_(v), line_(line) {}
    // Not explicit because do want implicit conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    Entity(const std::shared_ptr<Array> &v, size_t line = 0) : type_(EntityType::Arr), value_(v), line_(line) {}
    // Not explicit because do want implicit conversion
    // NOLINTNEXTLINE(google-explicit-constructor)
    Entity(const std::shared_ptr<Object> &v, size_t line = 0) : type_(EntityType::Obj), value_(v), line_(line) {}

    EntityType type() const { return type_; }

    size_t line() const { return line_; }

    Bool boolValue() const {
        ensureType(EntityType::Bool);
        return boost::any_cast<Bool>(value_);
    }

    Long longValue() const {
        ensureType(EntityType::Long);
        return boost::any_cast<Long>(value_);
    }

    Double doubleValue() const {
        ensureType(EntityType::Double);
        return boost::any_cast<Double>(value_);
    }

    String stringValue() const;

    String bytesValue() const;

    const Array &arrayValue() const {
        ensureType(EntityType::Arr);
        return **boost::any_cast<std::shared_ptr<Array>>(&value_);
    }

    const Object &objectValue() const {
        ensureType(EntityType::Obj);
        return **boost::any_cast<std::shared_ptr<Object>>(&value_);
    }

    std::string toString() const;
};

template<typename T>
struct type_traits {
};

template<>
struct type_traits<bool> {
    static EntityType type() { return EntityType::Bool; }
    static const char *name() { return "bool"; }
};

template<>
struct type_traits<int64_t> {
    static EntityType type() { return EntityType::Long; }
    static const char *name() { return "long"; }
};

template<>
struct type_traits<double> {
    static EntityType type() { return EntityType::Double; }
    static const char *name() { return "double"; }
};

template<>
struct type_traits<std::string> {
    static EntityType type() { return EntityType::String; }
    static const char *name() { return "string"; }
};

template<>
struct type_traits<std::vector<Entity>> {
    static EntityType type() { return EntityType::Arr; }
    static const char *name() { return "array"; }
};

template<>
struct type_traits<std::map<std::string, Entity>> {
    static EntityType type() { return EntityType::Obj; }
    static const char *name() { return "object"; }
};

AINGLE_DECL Entity readEntity(JsonParser &p);

AINGLE_DECL Entity loadEntity(InputStream &in);
AINGLE_DECL Entity loadEntity(const char *text);
AINGLE_DECL Entity loadEntity(const uint8_t *text, size_t len);

void writeEntity(JsonGenerator<JsonNullFormatter> &g, const Entity &n);

} // namespace json
} // namespace aingle

#endif
