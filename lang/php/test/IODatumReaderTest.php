<?php
/**

 */

namespace Apache\AIngle\Tests;

use Apache\AIngle\Datum\AIngleIOBinaryDecoder;
use Apache\AIngle\Datum\AIngleIOBinaryEncoder;
use Apache\AIngle\Datum\AIngleIODatumReader;
use Apache\AIngle\Datum\AIngleIODatumWriter;
use Apache\AIngle\IO\AIngleStringIO;
use Apache\AIngle\Schema\AIngleSchema;
use PHPUnit\Framework\TestCase;

class IODatumReaderTest extends TestCase
{
    public function testSchemaMatching()
    {
        $writers_schema = <<<JSON
      { "type": "map",
        "values": "bytes" }
JSON;
        $readers_schema = $writers_schema;
        $this->assertTrue(AIngleIODatumReader::schemasMatch(
            AIngleSchema::parse($writers_schema),
            AIngleSchema::parse($readers_schema)));
    }

    public function test_aliased()
    {
        $writers_schema = AIngleSchema::parse(<<<SCHEMA
{"type":"record", "name":"Rec1", "fields":[
{"name":"field1", "type":"int"}
]}
SCHEMA);
    $readers_schema = AIngleSchema::parse(<<<SCHEMA
      {"type":"record", "name":"Rec2", "aliases":["Rec1"], "fields":[
        {"name":"field2", "aliases":["field1"], "type":"int"}
      ]}
    SCHEMA);

        $io = new AIngleStringIO();
        $writer = new AIngleIODatumWriter();
        $writer->writeData($writers_schema, ['field1' => 1], new AIngleIOBinaryEncoder($io));

        $bin = $io->string();
        $reader = new AIngleIODatumReader();
        $record = $reader->readRecord(
            $writers_schema,
            $readers_schema,
            new AIngleIOBinaryDecoder(new AIngleStringIO($bin))
        );

        $this->assertEquals(['field2' => 1], $record);
    }

    public function testRecordNullField()
    {
        $schema_json = <<<_JSON
{"name":"member",
 "type":"record",
 "fields":[{"name":"one", "type":"int"},
           {"name":"two", "type":["null", "string"]}
           ]}
_JSON;

        $schema = AIngleSchema::parse($schema_json);
        $datum = array("one" => 1);

        $io = new AIngleStringIO();
        $writer = new AIngleIODatumWriter($schema);
        $encoder = new AIngleIOBinaryEncoder($io);
        $writer->write($datum, $encoder);
        $bin = $io->string();

        $this->assertSame('0200', bin2hex($bin));
    }
}
