/**

 */

#include <fstream>

#include "aingle/ValidSchema.hh"
#include "aingle/Compiler.hh"


int
main()
{
    std::ifstream in("cpx.json");

    aingle::ValidSchema cpxSchema;
    aingle::compileJsonSchema(in, cpxSchema);
}
