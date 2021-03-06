/**

 */

#include "Compiler.hh"
#include "bigrecord.hh"
#include "bigrecord_r.hh"
#include "tweet.hh"
#include "union_array_union.hh"
#include "union_map_union.hh"

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

void setRecord(testgen::RootRecord &myRecord) {
    uint8_t fixed[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    myRecord.mylong = 212;
    myRecord.nestedrecord.inval1 = std::numeric_limits<double>::min();
    myRecord.nestedrecord.inval2 = "hello world";
    myRecord.nestedrecord.inval3 = std::numeric_limits<int32_t>::max();

    myRecord.mymap["one"] = 100;
    myRecord.mymap["two"] = 200;

    myRecord.myarray.push_back(3434.9);
    myRecord.myarray.push_back(7343.9);
    myRecord.myarray.push_back(-63445.9);
    myRecord.myenum = testgen::ExampleEnum::one;

    map<string, int32_t> m;
    m["one"] = 1;
    m["two"] = 2;
    myRecord.myunion.set_map(m);

    vector<uint8_t> v;
    v.push_back(1);
    v.push_back(2);
    myRecord.anotherunion.set_bytes(v);

    myRecord.mybool = true;
    myRecord.anothernested.inval1 = std::numeric_limits<double>::max();
    myRecord.anothernested.inval2 = "goodbye world";
    myRecord.anothernested.inval3 = std::numeric_limits<int32_t>::min();
    memcpy(&myRecord.myfixed[0], fixed, myRecord.myfixed.size());
    myRecord.anotherint = 4534;
    myRecord.bytes.push_back(10);
    myRecord.bytes.push_back(20);
}

template<typename T1, typename T2>
void checkRecord(const T1 &r1, const T2 &r2) {
    BOOST_CHECK_EQUAL(r1.mylong, r2.mylong);
    BOOST_CHECK_EQUAL(r1.nestedrecord.inval1, r2.nestedrecord.inval1);
    BOOST_CHECK_EQUAL(r1.nestedrecord.inval2, r2.nestedrecord.inval2);
    BOOST_CHECK_EQUAL(r1.nestedrecord.inval3, r2.nestedrecord.inval3);
    BOOST_CHECK(r1.mymap == r2.mymap);
    BOOST_CHECK(r1.myarray == r2.myarray);
    BOOST_CHECK_EQUAL(r1.myunion.idx(), r2.myunion.idx());
    BOOST_CHECK(r1.myunion.get_map() == r2.myunion.get_map());
    BOOST_CHECK_EQUAL(r1.anotherunion.idx(), r2.anotherunion.idx());
    BOOST_CHECK(r1.anotherunion.get_bytes() == r2.anotherunion.get_bytes());
    BOOST_CHECK_EQUAL(r1.mybool, r2.mybool);
    BOOST_CHECK_EQUAL(r1.anothernested.inval1, r2.anothernested.inval1);
    BOOST_CHECK_EQUAL(r1.anothernested.inval2, r2.anothernested.inval2);
    BOOST_CHECK_EQUAL(r1.anothernested.inval3, r2.anothernested.inval3);
    BOOST_CHECK_EQUAL_COLLECTIONS(r1.myfixed.begin(), r1.myfixed.end(),
                                  r2.myfixed.begin(), r2.myfixed.end());
    BOOST_CHECK_EQUAL(r1.anotherint, r2.anotherint);
    BOOST_CHECK_EQUAL(r1.bytes.size(), r2.bytes.size());
    BOOST_CHECK_EQUAL_COLLECTIONS(r1.bytes.begin(), r1.bytes.end(),
                                  r2.bytes.begin(), r2.bytes.end());
    /**
     * Usually, comparing two different enums is not reliable. But here it fine because we
     * know the generated code and are merely checking if AIngle did the right job.
     * Also, converting enum into unsigned int is not always safe. There are cases there could be
     * truncation. Again, we have a controlled situation and it is safe here.
     */
    BOOST_CHECK_EQUAL(static_cast<unsigned int>(r1.myenum), static_cast<unsigned int>(r2.myenum));
}

void checkDefaultValues(const testgen_r::RootRecord &r) {
    BOOST_CHECK_EQUAL(r.withDefaultValue.s1, "\"sval\\u8352\"");
    BOOST_CHECK_EQUAL(r.withDefaultValue.i1, 99);
    BOOST_CHECK_CLOSE(r.withDefaultValue.d1, 5.67, 1e-10);
    BOOST_CHECK_EQUAL(r.myarraywithDefaultValue[0], 2);
    BOOST_CHECK_EQUAL(r.myarraywithDefaultValue[1], 3);
    BOOST_CHECK_EQUAL(r.myfixedwithDefaultValue.get_val()[0], 0x01);
    BOOST_CHECK_EQUAL(r.byteswithDefaultValue.get_bytes()[0], 0xff);
    BOOST_CHECK_EQUAL(r.byteswithDefaultValue.get_bytes()[1], 0xaa);
}

void testEncoding() {
    ValidSchema s;
    ifstream ifs("jsonschemas/bigrecord");
    compileJsonSchema(ifs, s);
    unique_ptr<OutputStream> os = memoryOutputStream();
    EncoderPtr e = validatingEncoder(s, binaryEncoder());
    e->init(*os);
    testgen::RootRecord t1;
    setRecord(t1);
    aingle::encode(*e, t1);
    e->flush();

    DecoderPtr d = validatingDecoder(s, binaryDecoder());
    unique_ptr<InputStream> is = memoryInputStream(*os);
    d->init(*is);
    testgen::RootRecord t2;
    aingle::decode(*d, t2);

    checkRecord(t2, t1);
}

void testResolution() {
    ValidSchema s_w;
    ifstream ifs_w("jsonschemas/bigrecord");
    compileJsonSchema(ifs_w, s_w);
    unique_ptr<OutputStream> os = memoryOutputStream();
    EncoderPtr e = validatingEncoder(s_w, binaryEncoder());
    e->init(*os);
    testgen::RootRecord t1;
    setRecord(t1);
    aingle::encode(*e, t1);
    e->flush();

    ValidSchema s_r;
    ifstream ifs_r("jsonschemas/bigrecord_r");
    compileJsonSchema(ifs_r, s_r);
    DecoderPtr dd = binaryDecoder();
    unique_ptr<InputStream> is = memoryInputStream(*os);
    dd->init(*is);
    DecoderPtr rd = resolvingDecoder(s_w, s_r, dd);
    testgen_r::RootRecord t2;
    aingle::decode(*rd, t2);

    checkRecord(t2, t1);
    checkDefaultValues(t2);

    //Re-use the resolving decoder to decode again.
    unique_ptr<InputStream> is1 = memoryInputStream(*os);
    rd->init(*is1);
    testgen_r::RootRecord t3;
    aingle::decode(*rd, t3);
    checkRecord(t3, t1);
    checkDefaultValues(t3);

    // Test serialization of default values.
    // Serialize to string then compile from string.
    std::ostringstream oss;
    s_r.toJson(oss);
    ValidSchema s_rs = aingle::compileJsonSchemaFromString(oss.str());

    std::unique_ptr<InputStream> is2 = memoryInputStream(*os);
    dd->init(*is2);
    rd = resolvingDecoder(s_w, s_rs, dd);
    testgen_r::RootRecord t4;
    aingle::decode(*rd, t4);
    checkDefaultValues(t4);

    std::ostringstream oss_r;
    std::ostringstream oss_rs;
    s_r.toJson(oss_r);
    s_rs.toJson(oss_rs);
    BOOST_CHECK_EQUAL(oss_r.str(), oss_rs.str());
}

void testNamespace() {
    ValidSchema s;
    ifstream ifs("jsonschemas/tweet");
    // basic compilation should work
    compileJsonSchema(ifs, s);
    // an AInglePoint was defined and then referred to from within a namespace
    testgen3::AInglePoint point;
    point.latitude = 42.3570;
    point.longitude = -71.1109;
    // set it in something that referred to it in the schema
    testgen3::_tweet_Union__1__ twPoint;
    twPoint.set_AInglePoint(point);
}

void setRecord(uau::r1 &r) {
}

void check(const uau::r1 &r1, const uau::r1 &r2) {
}

void setRecord(umu::r1 &r) {
}

void check(const umu::r1 &r1, const umu::r1 &r2) {
}

template<typename T>
struct schemaFilename {};
template<>
struct schemaFilename<uau::r1> {
    static const char value[];
};
const char schemaFilename<uau::r1>::value[] = "jsonschemas/union_array_union";
template<>
struct schemaFilename<umu::r1> {
    static const char value[];
};
const char schemaFilename<umu::r1>::value[] = "jsonschemas/union_map_union";

template<typename T>
void testEncoding2() {
    ValidSchema s;
    ifstream ifs(schemaFilename<T>::value);
    compileJsonSchema(ifs, s);

    unique_ptr<OutputStream> os = memoryOutputStream();
    EncoderPtr e = validatingEncoder(s, binaryEncoder());
    e->init(*os);
    T t1;
    setRecord(t1);
    aingle::encode(*e, t1);
    e->flush();

    DecoderPtr d = validatingDecoder(s, binaryDecoder());
    unique_ptr<InputStream> is = memoryInputStream(*os);
    d->init(*is);
    T t2;
    aingle::decode(*d, t2);

    check(t2, t1);
}

boost::unit_test::test_suite *
init_unit_test_suite(int /*argc*/, char * /*argv*/[]) {
    auto *ts = BOOST_TEST_SUITE("Code generator tests");
    ts->add(BOOST_TEST_CASE(testEncoding));
    ts->add(BOOST_TEST_CASE(testResolution));
    ts->add(BOOST_TEST_CASE(testEncoding2<uau::r1>));
    ts->add(BOOST_TEST_CASE(testEncoding2<umu::r1>));
    ts->add(BOOST_TEST_CASE(testNamespace));
    return ts;
}
