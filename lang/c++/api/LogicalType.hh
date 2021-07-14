/*

 */

#ifndef aingle_LogicalType_hh__
#define aingle_LogicalType_hh__

#include <iostream>

#include "Config.hh"

namespace aingle {

class AINGLE_DECL LogicalType {
public:
    enum Type {
        NONE,
        DECIMAL,
        DATE,
        TIME_MILLIS,
        TIME_MICROS,
        TIMESTAMP_MILLIS,
        TIMESTAMP_MICROS,
        DURATION,
        UUID
    };

    explicit LogicalType(Type type);

    Type type() const;

    // Precision and scale can only be set for the DECIMAL logical type.
    // Precision must be positive and scale must be either positive or zero. The
    // setters will throw an exception if they are called on any type other
    // than DECIMAL.
    void setPrecision(int precision);
    int precision() const { return precision_; }
    void setScale(int scale);
    int scale() const { return scale_; }

    void printJson(std::ostream &os) const;

private:
    Type type_;
    int precision_;
    int scale_;
};

} // namespace aingle

#endif
