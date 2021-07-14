/**

 */
#include "cpp_reserved_words.hh"

#include "Compiler.hh"

#include <boost/test/included/unit_test_framework.hpp>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using std::ifstream;
using std::map;
using std::string;
using std::unique_ptr;
using std::vector;

using aingle::binaryDecoder;
using aingle::binaryEncoder;
using aingle::Decoder;
using aingle::DecoderPtr;
using aingle::Encoder;
using aingle::EncoderPtr;
using aingle::InputStream;
using aingle::memoryInputStream;
using aingle::memoryOutputStream;
using aingle::OutputStream;
using aingle::validatingDecoder;
using aingle::validatingEncoder;
using aingle::ValidSchema;

void testCppReservedWords() {
    // Simply including the generated header is enough to test this.
    // the header will not compile if reserved words were used
}

boost::unit_test::test_suite *
init_unit_test_suite(int /* argc */, char * /*argv*/[]) {
    auto *ts = BOOST_TEST_SUITE("Code generator tests");
    ts->add(BOOST_TEST_CASE(testCppReservedWords));
    return ts;
}
