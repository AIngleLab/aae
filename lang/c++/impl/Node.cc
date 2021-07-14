/**

 */

#include <cmath>

#include "Node.hh"

namespace aingle {

using std::string;

Node::~Node() = default;

Name::Name(const std::string &name) {
    fullname(name);
}

string Name::fullname() const {
    return (ns_.empty()) ? simpleName_ : ns_ + "." + simpleName_;
}

void Name::fullname(const string &name) {
    string::size_type n = name.find_last_of('.');
    if (n == string::npos) {
        simpleName_ = name;
        ns_.clear();
    } else {
        ns_ = name.substr(0, n);
        simpleName_ = name.substr(n + 1);
    }
    check();
}

bool Name::operator<(const Name &n) const {
    return (ns_ < n.ns_) || (!(n.ns_ < ns_) && (simpleName_ < n.simpleName_));
}

static bool invalidChar1(char c) {
    return !isalnum(c) && c != '_' && c != '.' && c != '$';
}

static bool invalidChar2(char c) {
    return !isalnum(c) && c != '_';
}

void Name::check() const {
    if (!ns_.empty() && (ns_[0] == '.' || ns_[ns_.size() - 1] == '.' || std::find_if(ns_.begin(), ns_.end(), invalidChar1) != ns_.end())) {
        throw Exception("Invalid namespace: " + ns_);
    }
    if (simpleName_.empty()
        || std::find_if(simpleName_.begin(), simpleName_.end(), invalidChar2) != simpleName_.end()) {
        throw Exception("Invalid name: " + simpleName_);
    }
}

bool Name::operator==(const Name &n) const {
    return ns_ == n.ns_ && simpleName_ == n.simpleName_;
}

void Node::setLogicalType(LogicalType logicalType) {
    checkLock();

    // Check that the logical type is applicable to the node type.
    switch (logicalType.type()) {
        case LogicalType::NONE: break;
        case LogicalType::DECIMAL: {
            if (type_ != AINGLE_BYTES && type_ != AINGLE_FIXED) {
                throw Exception("DECIMAL logical type can annotate "
                                "only BYTES or FIXED type");
            }
            if (type_ == AINGLE_FIXED) {
                // Max precision that can be supported by the current size of
                // the FIXED type.
                long maxPrecision = floor(log10(2.0) * (8.0 * fixedSize() - 1));
                if (logicalType.precision() > maxPrecision) {
                    throw Exception(
                        boost::format(
                            "DECIMAL precision %1% is too large for the "
                            "FIXED type of size %2%, precision cannot be "
                            "larger than %3%")
                        % logicalType.precision() % fixedSize() % maxPrecision);
                }
            }
            if (logicalType.scale() > logicalType.precision()) {
                throw Exception("DECIMAL scale cannot exceed precision");
            }
            break;
        }
        case LogicalType::DATE:
            if (type_ != AINGLE_INT) {
                throw Exception("DATE logical type can only annotate INT type");
            }
            break;
        case LogicalType::TIME_MILLIS:
            if (type_ != AINGLE_INT) {
                throw Exception("TIME-MILLIS logical type can only annotate "
                                "INT type");
            }
            break;
        case LogicalType::TIME_MICROS:
            if (type_ != AINGLE_LONG) {
                throw Exception("TIME-MICROS logical type can only annotate "
                                "LONG type");
            }
            break;
        case LogicalType::TIMESTAMP_MILLIS:
            if (type_ != AINGLE_LONG) {
                throw Exception("TIMESTAMP-MILLIS logical type can only annotate "
                                "LONG type");
            }
            break;
        case LogicalType::TIMESTAMP_MICROS:
            if (type_ != AINGLE_LONG) {
                throw Exception("TIMESTAMP-MICROS logical type can only annotate "
                                "LONG type");
            }
            break;
        case LogicalType::DURATION:
            if (type_ != AINGLE_FIXED || fixedSize() != 12) {
                throw Exception("DURATION logical type can only annotate "
                                "FIXED type of size 12");
            }
            break;
        case LogicalType::UUID:
            if (type_ != AINGLE_STRING) {
                throw Exception("UUID logical type can only annotate "
                                "STRING type");
            }
            break;
    }

    logicalType_ = logicalType;
}

} // namespace aingle
