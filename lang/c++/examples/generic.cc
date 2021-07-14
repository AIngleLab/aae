/**

 */

#include <fstream>
#include <complex>

#include "cpx.hh"

#include "aingle/Compiler.hh"
#include "aingle/Encoder.hh"
#include "aingle/Decoder.hh"
#include "aingle/Specific.hh"
#include "aingle/Generic.hh"

int
main()
{
    std::ifstream ifs("cpx.json");

    aingle::ValidSchema cpxSchema;
    aingle::compileJsonSchema(ifs, cpxSchema);

    std::unique_ptr<aingle::OutputStream> out = aingle::memoryOutputStream();
    aingle::EncoderPtr e = aingle::binaryEncoder();
    e->init(*out);
    c::cpx c1;
    c1.re = 100.23;
    c1.im = 105.77;
    aingle::encode(*e, c1);

    std::unique_ptr<aingle::InputStream> in = aingle::memoryInputStream(*out);
    aingle::DecoderPtr d = aingle::binaryDecoder();
    d->init(*in);

    aingle::GenericDatum datum(cpxSchema);
    aingle::decode(*d, datum);
    std::cout << "Type: " << datum.type() << std::endl;
    if (datum.type() == aingle::AINGLE_RECORD) {
        const aingle::GenericRecord& r = datum.value<aingle::GenericRecord>();
        std::cout << "Field-count: " << r.fieldCount() << std::endl;
        if (r.fieldCount() == 2) {
            const aingle::GenericDatum& f0 = r.fieldAt(0);
            if (f0.type() == aingle::AINGLE_DOUBLE) {
                std::cout << "Real: " << f0.value<double>() << std::endl;
            }
            const aingle::GenericDatum& f1 = r.fieldAt(1);
            if (f1.type() == aingle::AINGLE_DOUBLE) {
                std::cout << "Imaginary: " << f1.value<double>() << std::endl;
            }
        }
    }
    return 0;
}
