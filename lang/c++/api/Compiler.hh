/*

 */

#ifndef aingle_Compiler_hh__
#define aingle_Compiler_hh__

#include "Config.hh"
#include <cstdint>
#include <istream>

namespace aingle {

class AINGLE_DECL InputStream;

/// This class is used to implement an aingle spec parser using a flex/bison
/// compiler.  In order for the lexer to be reentrant, this class provides a
/// lexer object for each parse.  The bison parser also uses this class to
/// build up an aingle parse tree as the aingle spec is parsed.

class AINGLE_DECL ValidSchema;

/// Given a stream containing a JSON schema, compiles the schema to a
/// ValidSchema object.  Throws if the schema cannot be compiled to a valid
/// schema

AINGLE_DECL void compileJsonSchema(std::istream &is, ValidSchema &schema);

/// Non-throwing version of compileJsonSchema.
///
/// \return True if no error, false if error (with the error string set)
///

AINGLE_DECL bool compileJsonSchema(std::istream &is, ValidSchema &schema,
                                 std::string &error);

AINGLE_DECL ValidSchema compileJsonSchemaFromStream(InputStream &is);

AINGLE_DECL ValidSchema compileJsonSchemaFromMemory(const uint8_t *input, size_t len);

AINGLE_DECL ValidSchema compileJsonSchemaFromString(const char *input);

AINGLE_DECL ValidSchema compileJsonSchemaFromString(const std::string &input);

AINGLE_DECL ValidSchema compileJsonSchemaFromFile(const char *filename);

} // namespace aingle

#endif
