<?php

/**

 */

namespace Apache\AIngle\DataFile;

use Apache\AIngle\AIngleIO;
use Apache\AIngle\Datum\AIngleIODatumReader;
use Apache\AIngle\Datum\AIngleIODatumWriter;
use Apache\AIngle\IO\AIngleFile;
use Apache\AIngle\Schema\AIngleSchema;

/**
 * @package AIngle
 */
class AIngleDataIO
{
    /**
     * @var int used in file header
     */
    public const VERSION = 1;

    /**
     * @var int count of bytes in synchronization marker
     */
    public const SYNC_SIZE = 16;

    /**
     * @var int   count of items per block, arbitrarily set to 4000 * SYNC_SIZE
     * @todo make this value configurable
     */
    public const SYNC_INTERVAL = 64000;

    /**
     * @var string map key for datafile metadata codec value
     */
    public const METADATA_CODEC_ATTR = 'aingle.codec';

    /**
     * @var string map key for datafile metadata schema value
     */
    public const METADATA_SCHEMA_ATTR = 'aingle.schema';
    /**
     * @var string JSON for datafile metadata schema
     */
    public const METADATA_SCHEMA_JSON = '{"type":"map","values":"bytes"}';

    /**
     * @var string codec value for NULL codec
     */
    public const NULL_CODEC = 'null';

    /**
     * @var string codec value for deflate codec
     */
    public const DEFLATE_CODEC = 'deflate';

    public const SNAPPY_CODEC = 'snappy';

    public const ZSTANDARD_CODEC = 'zstandard';

    public const BZIP2_CODEC = 'bzip2';

    /**
     * @var array array of valid codec names
     */
    private static $validCodecs = [
        self::NULL_CODEC,
        self::DEFLATE_CODEC,
        self::SNAPPY_CODEC,
        self::ZSTANDARD_CODEC,
        self::BZIP2_CODEC
    ];

    /**
     * @var AIngleSchema cached version of metadata schema object
     */
    private static $metadataSchema;

    /**
     * @returns int count of bytes in the initial "magic" segment of the
     *              AIngle container file header
     */
    public static function magicSize()
    {
        return strlen(self::magic());
    }

    /**
     * @returns the initial "magic" segment of an AIngle container file header.
     */
    public static function magic()
    {
        return ('Obj' . pack('c', self::VERSION));
    }

    /**
     * @returns AIngleSchema object of AIngle container file metadata.
     */
    public static function metadataSchema()
    {
        if (is_null(self::$metadataSchema)) {
            self::$metadataSchema = AIngleSchema::parse(self::METADATA_SCHEMA_JSON);
        }
        return self::$metadataSchema;
    }

    /**
     * @param string $file_path file_path of file to open
     * @param string $mode one of AIngleFile::READ_MODE or AIngleFile::WRITE_MODE
     * @param string $schemaJson JSON of writer's schema
     * @param string $codec compression codec
     * @returns AIngleDataIOWriter instance of AIngleDataIOWriter
     *
     * @throws AIngleDataIOException if $writers_schema is not provided
     *         or if an invalid $mode is given.
     */
    public static function openFile(
        $file_path,
        $mode = AIngleFile::READ_MODE,
        $schemaJson = null,
        $codec = self::NULL_CODEC
    ) {
        $schema = !is_null($schemaJson)
            ? AIngleSchema::parse($schemaJson) : null;

        $io = false;
        switch ($mode) {
            case AIngleFile::WRITE_MODE:
                if (is_null($schema)) {
                    throw new AIngleDataIOException('Writing an AIngle file requires a schema.');
                }
                $file = new AIngleFile($file_path, AIngleFile::WRITE_MODE);
                $io = self::openWriter($file, $schema, $codec);
                break;
            case AIngleFile::READ_MODE:
                $file = new AIngleFile($file_path, AIngleFile::READ_MODE);
                $io = self::openReader($file, $schema);
                break;
            default:
                throw new AIngleDataIOException(
                    sprintf(
                        "Only modes '%s' and '%s' allowed. You gave '%s'.",
                        AIngleFile::READ_MODE,
                        AIngleFile::WRITE_MODE,
                        $mode
                    )
                );
        }
        return $io;
    }

    /**
     * @param AIngleIO $io
     * @param AIngleSchema $schema
     * @param string $codec
     * @returns AIngleDataIOWriter
     */
    protected static function openWriter($io, $schema, $codec = self::NULL_CODEC)
    {
        $writer = new AIngleIODatumWriter($schema);
        return new AIngleDataIOWriter($io, $writer, $schema, $codec);
    }

    /**
     * @param AIngleIO $io
     * @param AIngleSchema $schema
     * @returns AIngleDataIOReader
     */
    protected static function openReader($io, $schema)
    {
        $reader = new AIngleIODatumReader(null, $schema);
        return new AIngleDataIOReader($io, $reader);
    }

    /**
     * @param string $codec
     * @returns boolean true if $codec is a valid codec value and false otherwise
     */
    public static function isValidCodec($codec)
    {
        return in_array($codec, self::validCodecs());
    }

    /**
     * @returns array array of valid codecs
     */
    public static function validCodecs()
    {
        return self::$validCodecs;
    }
}
