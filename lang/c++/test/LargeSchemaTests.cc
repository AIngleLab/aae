/**

 */

#include "Compiler.hh"
#include "Decoder.hh"
#include "ValidSchema.hh"
#include <fstream>

#include <boost/test/included/unit_test_framework.hpp>
#include <boost/test/parameterized_test.hpp>

void testLargeSchema() {
    std::ifstream in("jsonschemas/large_schema.ain");
    aingle::ValidSchema vs;
    aingle::compileJsonSchema(in, vs);
    aingle::DecoderPtr d = aingle::binaryDecoder();
    aingle::DecoderPtr vd = aingle::validatingDecoder(vs, d);
    aingle::DecoderPtr rd = aingle::resolvingDecoder(vs, vs, d);
}

boost::unit_test::test_suite *
init_unit_test_suite(int /*argc*/, char * /*argv*/[]) {
    using namespace boost::unit_test;

    auto *ts = BOOST_TEST_SUITE("AIngle C++ unit tests for schemas");
    ts->add(BOOST_TEST_CASE(&testLargeSchema));
    return ts;
}
