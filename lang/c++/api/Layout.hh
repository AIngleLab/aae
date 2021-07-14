/*

 */

#ifndef aingle_Layout_hh__
#define aingle_Layout_hh__

#include "Config.hh"
#include <boost/noncopyable.hpp>

/// \file Layout.hh
///

namespace aingle {

class AINGLE_DECL Layout : private boost::noncopyable {
protected:
    explicit Layout(size_t offset = 0) : offset_(offset) {}

public:
    size_t offset() const {
        return offset_;
    }
    virtual ~Layout() = default;

private:
    const size_t offset_;
};

class AINGLE_DECL PrimitiveLayout : public Layout {
public:
    explicit PrimitiveLayout(size_t offset = 0) : Layout(offset) {}
};

class AINGLE_DECL CompoundLayout : public Layout {

public:
    explicit CompoundLayout(size_t offset = 0) : Layout(offset) {}

    void add(std::unique_ptr<Layout> &layout) {
        layouts_.push_back(std::move(layout));
    }

    const Layout &at(size_t idx) const {
        return *layouts_.at(idx);
    }

private:
    std::vector<std::unique_ptr<Layout>> layouts_;
};

} // namespace aingle

#endif
