<?php
/**

 */

namespace Apache\AIngle\Tests;

use Apache\AIngle\AIngleDebug;
use Apache\AIngle\AIngleIO;
use Apache\AIngle\DataFile\AIngleDataIOReader;
use Apache\AIngle\DataFile\AIngleDataIOWriter;
use Apache\AIngle\Datum\AIngleIODatumReader;
use Apache\AIngle\Datum\AIngleIODatumWriter;
use Apache\AIngle\IO\AIngleStringIO;
use Apache\AIngle\Schema\AIngleSchema;
use PHPUnit\Framework\TestCase;

class StringIOTest extends TestCase
{
    public function test_write()
    {
        $strio = new AIngleStringIO();
        $this->assertEquals(0, $strio->tell());
        $str = 'foo';
        $strlen = strlen($str);
        $this->assertEquals($strlen, $strio->write($str));
        $this->assertEquals($strlen, $strio->tell());
    }

    public function test_seek()
    {
        $strio = new AIngleStringIO('abcdefghijklmnopqrstuvwxyz');
        $strio->seek(4, AIngleIO::SEEK_SET);
        $this->assertEquals('efgh', $strio->read(4));
        $strio->seek(4, AIngleIO::SEEK_CUR);
        $this->assertEquals('mnop', $strio->read(4));
        $strio->seek(-4, AIngleIO::SEEK_END);
        $this->assertEquals('wxyz', $strio->read(4));
    }

    public function test_tell()
    {
        $strio = new AIngleStringIO('foobar');
        $this->assertEquals(0, $strio->tell());
        $strlen = 3;
        $strio->read($strlen);
        $this->assertEquals($strlen, $strio->tell());
    }

    public function test_read()
    {
        $str = 'foobar';
        $strio = new AIngleStringIO($str);
        $strlen = 3;
        $this->assertEquals(substr($str, 0, $strlen), $strio->read($strlen));
    }

    public function test_string_rep()
    {
        $writers_schema_json = '"null"';
        $writers_schema = AIngleSchema::parse($writers_schema_json);
        $datum_writer = new AIngleIODatumWriter($writers_schema);
        $strio = new AIngleStringIO();
        $this->assertEquals('', $strio->string());
        $dw = new AIngleDataIOWriter($strio, $datum_writer, $writers_schema_json);
        $dw->close();

        $this->assertEquals(57, strlen($strio->string()),
            AIngleDebug::asciiString($strio->string()));

        $read_strio = new AIngleStringIO($strio->string());

        $datum_reader = new AIngleIODatumReader();
        $dr = new AIngleDataIOReader($read_strio, $datum_reader);
        $read_data = $dr->data();
        $datum_count = count($read_data);
        $this->assertEquals(0, $datum_count);
    }
}
