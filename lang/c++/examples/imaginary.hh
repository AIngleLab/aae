/**

 */


#ifndef IMAGINARY_HH_3460301992__H_
#define IMAGINARY_HH_3460301992__H_


#include "boost/any.hpp"
#include "aingle/Specific.hh"
#include "aingle/Encoder.hh"
#include "aingle/Decoder.hh"

namespace i {
struct cpx {
    double im;
};

}
namespace aingle {
template<> struct codec_traits<i::cpx> {
    static void encode(Encoder& e, const i::cpx& v) {
        aingle::encode(e, v.im);
    }
    static void decode(Decoder& d, i::cpx& v) {
        aingle::decode(d, v.im);
    }
};

}
#endif
