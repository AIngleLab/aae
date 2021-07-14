/*

 */

#ifndef aingle_Validating_hh__
#define aingle_Validating_hh__

#include <boost/noncopyable.hpp>
#include <cstdint>
#include <utility>
#include <vector>

#include "Config.hh"
#include "Types.hh"
#include "ValidSchema.hh"

namespace aingle {

class AINGLE_DECL NullValidator : private boost::noncopyable {
public:
    explicit NullValidator(const ValidSchema &schema) {}
    NullValidator() = default;

    void setCount(int64_t) {}

    static bool typeIsExpected(Type) {
        return true;
    }

    static Type nextTypeExpected() {
        return AINGLE_UNKNOWN;
    }

    static int nextSizeExpected() {
        return 0;
    }

    static bool getCurrentRecordName(std::string &name) {
        return true;
    }

    static bool getNextFieldName(std::string &name) {
        return true;
    }

    void checkTypeExpected(Type) {}
    void checkFixedSizeExpected(int) {}
};

/// This class is used by both the ValidatingSerializer and ValidationParser
/// objects.  It advances the parse tree (containing logic how to advance
/// through the various compound types, for example a record must advance
/// through all leaf nodes but a union only skips to one), and reports which
/// type is next.

class AINGLE_DECL Validator : private boost::noncopyable {
public:
    explicit Validator(ValidSchema schema);

    void setCount(int64_t val);

    bool typeIsExpected(Type type) const {
        return (expectedTypesFlag_ & typeToFlag(type)) != 0;
    }

    Type nextTypeExpected() const {
        return nextType_;
    }

    int nextSizeExpected() const;

    bool getCurrentRecordName(std::string &name) const;
    bool getNextFieldName(std::string &name) const;

    void checkTypeExpected(Type type) {
        if (!typeIsExpected(type)) {
            throw Exception(
                boost::format("Type %1% does not match schema %2%")
                % type % nextType_);
        }
        advance();
    }

    void checkFixedSizeExpected(int size) {
        if (nextSizeExpected() != size) {
            throw Exception(
                boost::format("Wrong size for fixed, got %1%, expected %2%")
                % size % nextSizeExpected());
        }
        checkTypeExpected(AINGLE_FIXED);
    }

private:
    using flag_t = uint32_t;

    static flag_t typeToFlag(Type type) {
        flag_t flag = (1L << type);
        return flag;
    }

    void setupOperation(const NodePtr &node);

    void setWaitingForCount();

    void advance();
    void doAdvance();

    void enumAdvance();
    bool countingSetup();
    void countingAdvance();
    void unionAdvance();
    void fixedAdvance();

    void setupFlag(Type type);

    const ValidSchema schema_;

    Type nextType_;
    flag_t expectedTypesFlag_;
    bool compoundStarted_;
    bool waitingForCount_;
    int64_t count_;

    struct CompoundType {
        explicit CompoundType(NodePtr n) : node(std::move(n)), pos(0) {}
        NodePtr node; ///< save the node
        size_t pos;   ///< track the leaf position to visit
    };

    std::vector<CompoundType> compoundStack_;
    std::vector<size_t> counters_;
};

} // namespace aingle

#endif
