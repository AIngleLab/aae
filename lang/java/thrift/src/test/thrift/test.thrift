/**

 */

namespace java org.apache.aingle.thrift.test

enum E {
  X = 1,
  Y = 2,
  Z = 3,
}

struct Nested {
  1: i32 x
}

union FooOrBar {
  1: string foo;
  2: string bar;
}


// contains each primitive type
struct Test {
  1: bool boolField
  2: byte byteField
 16: optional byte byteOptionalField
  3: i16 i16Field
 15: optional i16 i16OptionalField
  4: optional i32 i32Field
  5: i64 i64Field
  6: double doubleField
  7: string stringField
  8: optional binary binaryField
  9: map<string,i32> mapField
 10: list<i32> listField
 11: set<i32> setField
 12: E enumField
 13: Nested structField
 14: FooOrBar fooOrBar
}

exception Error {
  1: string message,
}

service Foo {

   void ping(),

   i32 add(1:i32 num1, 2:i32 num2),

   oneway void zip(),
}
