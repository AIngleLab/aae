/*

 */

#ifndef aingle_Exception_hh__
#define aingle_Exception_hh__

#include "Config.hh"
#include <boost/format.hpp>
#include <stdexcept>

namespace aingle {

/// Wrapper for std::runtime_error that provides convenience constructor
/// for boost::format objects

class AINGLE_DECL Exception : public virtual std::runtime_error {
public:
    explicit Exception(const std::string &msg) : std::runtime_error(msg) {}

    explicit Exception(const boost::format &msg) : std::runtime_error(boost::str(msg)) {}
};

} // namespace aingle

#endif
