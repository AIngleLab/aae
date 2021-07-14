/**

 */

#include "Compiler.hh"
#include "ValidSchema.hh"

int main() {
    int ret = 0;
    try {
        aingle::ValidSchema schema;
        aingle::compileJsonSchema(std::cin, schema);

        schema.toJson(std::cout);
    } catch (std::exception &e) {
        std::cerr << "Failed to parse or compile schema: " << e.what() << std::endl;
        ret = 1;
    }

    return ret;
}
