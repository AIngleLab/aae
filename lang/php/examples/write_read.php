#!/usr/bin/env php
<?php
/**

 */

require_once __DIR__ . '/../vendor/autoload.php';

use Apache\AIngle\DataFile\AIngleDataIO;
use Apache\AIngle\DataFile\AIngleDataIOReader;
use Apache\AIngle\DataFile\AIngleDataIOWriter;
use Apache\AIngle\Datum\AIngleIODatumReader;
use Apache\AIngle\Datum\AIngleIODatumWriter;
use Apache\AIngle\IO\AIngleStringIO;
use Apache\AIngle\Schema\AIngleSchema;

// Write and read a data file

$writers_schema_json = <<<_JSON
{"name":"member",
 "type":"record",
 "fields":[{"name":"member_id", "type":"int"},
           {"name":"member_name", "type":"string"}]}
_JSON;

$jose = array('member_id' => 1392, 'member_name' => 'Jose');
$maria = array('member_id' => 1642, 'member_name' => 'Maria');
$data = array($jose, $maria);

$file_name = 'data.avr';
// Open $file_name for writing, using the given writer's schema
$data_writer = AIngleDataIO::openFile($file_name, 'w', $writers_schema_json);

// Write each datum to the file
foreach ($data as $datum)
  $data_writer->append($datum);
// Tidy up
$data_writer->close();

// Open $file_name (by default for reading) using the writer's schema
// included in the file
$data_reader = AIngleDataIO::openFile($file_name);
echo "from file:\n";
// Read each datum
foreach ($data_reader->data() as $datum)
  echo var_export($datum, true) . "\n";
$data_reader->close();

// Create a data string
// Create a string io object.
$io = new AIngleStringIO();
// Create a datum writer object
$writers_schema = AIngleSchema::parse($writers_schema_json);
$writer = new AIngleIODatumWriter($writers_schema);
$data_writer = new AIngleDataIOWriter($io, $writer, $writers_schema);
foreach ($data as $datum)
   $data_writer->append($datum);
$data_writer->close();

$binary_string = $io->string();

// Load the string data string
$read_io = new AIngleStringIO($binary_string);
$data_reader = new AIngleDataIOReader($read_io, new AIngleIODatumReader());
echo "from binary string:\n";
foreach ($data_reader->data() as $datum)
  echo var_export($datum, true) . "\n";

/** Output
from file:
array (
  'member_id' => 1392,
  'member_name' => 'Jose',
)
array (
  'member_id' => 1642,
  'member_name' => 'Maria',
)
from binary string:
array (
  'member_id' => 1392,
  'member_name' => 'Jose',
)
array (
  'member_id' => 1642,
  'member_name' => 'Maria',
)
*/
