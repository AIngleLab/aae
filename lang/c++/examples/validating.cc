/**

 */

#include <fstream>
#include <complex>

#include "aingle/Compiler.hh"
#include "aingle/Encoder.hh"
#include "aingle/Decoder.hh"
#include "aingle/Specific.hh"

namespace aingle {
template<typename T>
struct codec_traits<std::complex<T> > {
    static void encode(Encoder& e, const std::complex<T>& c) {
        aingle::encode(e, std::real(c));
        aingle::encode(e, std::imag(c));
    }

    static void decode(Decoder& d, std::complex<T>& c) {
        T re, im;
        aingle::decode(d, re);
        aingle::decode(d, im);
        c = std::complex<T>(re, im);
    }
};

}
int
main()
{
    std::ifstream ifs("cpx.json");

    aingle::ValidSchema cpxSchema;
    aingle::compileJsonSchema(ifs, cpxSchema);

    std::unique_ptr<aingle::OutputStream> out = aingle::memoryOutputStream();
    aingle::EncoderPtr e = aingle::validatingEncoder(cpxSchema,
        aingle::binaryEncoder());
    e->init(*out);
    std::complex<double> c1(1.0, 2.0);
    aingle::encode(*e, c1);

    std::unique_ptr<aingle::InputStream> in = aingle::memoryInputStream(*out);
    aingle::DecoderPtr d = aingle::validatingDecoder(cpxSchema,
        aingle::binaryDecoder());
    d->init(*in);

    std::complex<double> c2;
    aingle::decode(*d, c2);
    std::cout << '(' << std::real(c2) << ", " << std::imag(c2) << ')' << std::endl;
    return 0;
}
