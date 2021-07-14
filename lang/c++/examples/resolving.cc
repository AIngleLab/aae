/**

 */

#include <fstream>

#include "cpx.hh"
#include "imaginary.hh"

#include "aingle/Compiler.hh"
#include "aingle/Encoder.hh"
#include "aingle/Decoder.hh"
#include "aingle/Specific.hh"
#include "aingle/Generic.hh"



aingle::ValidSchema load(const char* filename)
{
    std::ifstream ifs(filename);
    aingle::ValidSchema result;
    aingle::compileJsonSchema(ifs, result);
    return result;
}

int
main()
{
    aingle::ValidSchema cpxSchema = load("cpx.json");
    aingle::ValidSchema imaginarySchema = load("imaginary.json");

    std::unique_ptr<aingle::OutputStream> out = aingle::memoryOutputStream();
    aingle::EncoderPtr e = aingle::binaryEncoder();
    e->init(*out);
    c::cpx c1;
    c1.re = 100.23;
    c1.im = 105.77;
    aingle::encode(*e, c1);

    std::unique_ptr<aingle::InputStream> in = aingle::memoryInputStream(*out);
    aingle::DecoderPtr d = aingle::resolvingDecoder(cpxSchema, imaginarySchema,
        aingle::binaryDecoder());
    d->init(*in);

    i::cpx c2;
    aingle::decode(*d, c2);
    std::cout << "Imaginary: " << c2.im << std::endl;

}
