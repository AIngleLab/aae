/**

 */

#include "cpx.hh"
#include "aingle/Encoder.hh"
#include "aingle/Decoder.hh"


int
main()
{
    std::unique_ptr<aingle::OutputStream> out = aingle::memoryOutputStream();
    aingle::EncoderPtr e = aingle::binaryEncoder();
    e->init(*out);
    c::cpx c1;
    c1.re = 1.0;
    c1.im = 2.13;
    aingle::encode(*e, c1);

    std::unique_ptr<aingle::InputStream> in = aingle::memoryInputStream(*out);
    aingle::DecoderPtr d = aingle::binaryDecoder();
    d->init(*in);

    c::cpx c2;
    aingle::decode(*d, c2);
    std::cout << '(' << c2.re << ", " << c2.im << ')' << std::endl;
    return 0;
}

