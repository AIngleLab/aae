/**

 */

#include "LogicalType.hh"
#include "Exception.hh"

namespace aingle {

LogicalType::LogicalType(Type type)
    : type_(type), precision_(0), scale_(0) {}

LogicalType::Type LogicalType::type() const {
    return type_;
}

void LogicalType::setPrecision(int precision) {
    if (type_ != DECIMAL) {
        throw Exception("Only logical type DECIMAL can have precision");
    }
    if (precision <= 0) {
        throw Exception(boost::format("Precision cannot be: %1%") % precision);
    }
    precision_ = precision;
}

void LogicalType::setScale(int scale) {
    if (type_ != DECIMAL) {
        throw Exception("Only logical type DECIMAL can have scale");
    }
    if (scale < 0) {
        throw Exception(boost::format("Scale cannot be: %1%") % scale);
    }
    scale_ = scale;
}

void LogicalType::printJson(std::ostream &os) const {
    switch (type_) {
        case LogicalType::NONE: break;
        case LogicalType::DECIMAL:
            os << R"("logicalType": "decimal")";
            os << ", \"precision\": " << precision_;
            os << ", \"scale\": " << scale_;
            break;
        case DATE:
            os << R"("logicalType": "date")";
            break;
        case TIME_MILLIS:
            os << R"("logicalType": "time-millis")";
            break;
        case TIME_MICROS:
            os << R"("logicalType": "time-micros")";
            break;
        case TIMESTAMP_MILLIS:
            os << R"("logicalType": "timestamp-millis")";
            break;
        case TIMESTAMP_MICROS:
            os << R"("logicalType": "timestamp-micros")";
            break;
        case DURATION:
            os << R"("logicalType": "duration")";
            break;
        case UUID:
            os << R"("logicalType": "uuid")";
            break;
    }
}

} // namespace aingle
