/**

 */

#include <sstream>

#include <boost/test/included/unit_test_framework.hpp>
#include <boost/test/unit_test.hpp>

#include "Compiler.hh"
#include "ValidSchema.hh"

// Assert that empty defaults don't make json schema compilation violate bounds
// checks, as they did in AINGLE-1853. Please note that on Linux bounds are only
// checked in Debug builds (CMAKE_BUILD_TYPE=Debug).
void testEmptyBytesDefault() {
    std::string input = "{\n\
    \"type\": \"record\",\n\
    \"name\": \"testrecord\",\n\
    \"fields\": [\n\
        {\n\
            \"name\": \"testbytes\",\n\
            \"type\": \"bytes\",\n\
            \"default\": \"\"\n\
        }\n\
    ]\n\
}\n\
";
    std::string expected = "{\n\
    \"type\": \"record\",\n\
    \"name\": \"testrecord\",\n\
    \"fields\": [\n\
        {\n\
            \"name\": \"testbytes\",\n\
            \"type\": \"bytes\",\n\
            \"default\": \"\"\n\
        }\n\
    ]\n\
}\n\
";

    aingle::ValidSchema schema = aingle::compileJsonSchemaFromString(input);
    std::ostringstream actual;
    schema.toJson(actual);
    BOOST_CHECK_EQUAL(expected, actual.str());
}

void test2dArray() {
    std::string input = "{\n\
    \"type\": \"array\",\n\
    \"items\": {\n\
        \"type\": \"array\",\n\
        \"items\": \"double\"\n\
    }\n\
}\n";

    std::string expected = "{\n\
    \"type\": \"array\",\n\
    \"items\": {\n\
        \"type\": \"array\",\n\
        \"items\": \"double\"\n\
    }\n\
}\n\
";
    aingle::ValidSchema schema = aingle::compileJsonSchemaFromString(input);
    std::ostringstream actual;
    schema.toJson(actual);
    BOOST_CHECK_EQUAL(expected, actual.str());
}

boost::unit_test::test_suite *
init_unit_test_suite(int /*argc*/, char * /*argv*/[]) {
    using namespace boost::unit_test;

    auto *ts = BOOST_TEST_SUITE("AIngle C++ unit tests for Compiler.cc");
    ts->add(BOOST_TEST_CASE(&testEmptyBytesDefault));
    ts->add(BOOST_TEST_CASE(&test2dArray));
    return ts;
}
