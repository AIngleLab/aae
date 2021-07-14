/**

 */


#ifndef CPX_HH_1278398428__H_
#define CPX_HH_1278398428__H_


#include "aingle/Specific.hh"
#include "aingle/Encoder.hh"
#include "aingle/Decoder.hh"

namespace c {
struct cpx {
    double re;
    double im;
};

}
namespace aingle {
template<> struct codec_traits<c::cpx> {
    static void encode(Encoder& e, const c::cpx& v) {
        aingle::encode(e, v.re);
        aingle::encode(e, v.im);
    }
    static void decode(Decoder& d, c::cpx& v) {
        aingle::decode(d, v.re);
        aingle::decode(d, v.im);
    }
};

}
#endif
