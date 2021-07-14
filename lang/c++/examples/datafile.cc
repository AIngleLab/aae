/**

 */

#include <fstream>

#include "cpx.hh"
#include "aingle/Encoder.hh"
#include "aingle/Decoder.hh"
#include "aingle/ValidSchema.hh"
#include "aingle/Compiler.hh"
#include "aingle/DataFile.hh"


aingle::ValidSchema loadSchema(const char* filename)
{
    std::ifstream ifs(filename);
    aingle::ValidSchema result;
    aingle::compileJsonSchema(ifs, result);
    return result;
}

int
main()
{
    aingle::ValidSchema cpxSchema = loadSchema("cpx.json");

    {
        aingle::DataFileWriter<c::cpx> dfw("test.bin", cpxSchema);
        c::cpx c1;
        for (int i = 0; i < 100; i++) {
            c1.re = i * 100;
            c1.im = i + 100;
            dfw.write(c1);
        }
        dfw.close();
    }

    {
        aingle::DataFileReader<c::cpx> dfr("test.bin", cpxSchema);
        c::cpx c2;
        while (dfr.read(c2)) {
            std::cout << '(' << c2.re << ", " << c2.im << ')' << std::endl;
        }
    }
    return 0;
}

