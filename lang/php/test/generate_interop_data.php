#!/usr/bin/env php
<?php
/**

 */

require_once __DIR__ . '/test_helper.php';

use Apache\AIngle\DataFile\AIngleDataIO;

$datum = [
    'nullField' => null,
    'boolField' => true,
    'intField' => -42,
    'longField' => (int) 2147483650,
    'floatField' => 1234.0,
    'doubleField' => -5432.6,
    'stringField' => 'hello aingle',
    'bytesField' => "\x16\xa6",
    'arrayField' => [5.0, -6.0, -10.5],
    'mapField' => [
        'a' => ['label' => 'a'],
        'c' => ['label' => '3P0']
    ],
    'unionField' => 14.5,
    'enumField' => 'C',
    'fixedField' => '1019181716151413',
    'recordField' => [
        'label' => 'blah',
        'children' => [
            [
                'label' => 'inner',
                'children' => []
            ]
        ]
    ]
];

$schema_json = file_get_contents(AINGLE_INTEROP_SCHEMA);
foreach (AIngleDataIO::validCodecs() as $codec) {
    $file_name = $codec == AIngleDataIO::NULL_CODEC ? 'php.aingle' : sprintf('php_%s.aingle', $codec);
    $data_file = implode(DIRECTORY_SEPARATOR, [AINGLE_BUILD_DATA_DIR, $file_name]);
    $io_writer = AIngleDataIO::openFile($data_file, 'w', $schema_json, $codec);
    $io_writer->append($datum);
    $io_writer->close();
}
