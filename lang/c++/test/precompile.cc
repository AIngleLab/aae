/**

 */

#include <fstream>
#include <iostream>

#include "Compiler.hh"
#include "ValidSchema.hh"

int main(int argc, char **argv) {
    int ret = 0;
    try {
        aingle::ValidSchema schema;
        if (argc > 1) {
            std::ifstream in(argv[1]);
            aingle::compileJsonSchema(in, schema);
        } else {
            aingle::compileJsonSchema(std::cin, schema);
        }

        if (argc > 2) {
            std::ofstream out(argv[2]);
            schema.toFlatList(out);
        } else {
            schema.toFlatList(std::cout);
        }
    } catch (std::exception &e) {
        std::cerr << "Failed to parse or compile schema: " << e.what() << std::endl;
        ret = 1;
    }

    return ret;
}
