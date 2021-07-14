<?php

/**

 */

namespace Apache\AIngle\DataFile;

use Apache\AIngle\AIngleException;
use Apache\AIngle\AIngleIO;
use Apache\AIngle\Datum\AIngleIOBinaryEncoder;
use Apache\AIngle\Datum\AIngleIODatumReader;
use Apache\AIngle\Datum\AIngleIODatumWriter;
use Apache\AIngle\IO\AIngleStringIO;
use Apache\AIngle\Schema\AIngleSchema;

/**
 * Writes AIngle data to an AIngleIO source using an AIngleSchema
 * @package AIngle
 */
class AIngleDataIOWriter
{
    /**
     * @var AIngleIO object container where data is written
     */
    private $io;
    /**
     * @var AIngleIOBinaryEncoder encoder for object container
     */
    private $encoder;
    /**
     * @var AIngleIODatumWriter
     */
    private $datum_writer;
    /**
     * @var AIngleStringIO buffer for writing
     */
    private $buffer;
    /**
     * @var AIngleIOBinaryEncoder encoder for buffer
     */
    private $buffer_encoder;
    /**
     * @var int count of items written to block
     */
    private $block_count; // AIngleIOBinaryEncoder
    /**
     * @var array map of object container metadata
     */
    private $metadata;
    /**
     * @var string compression codec
     */
    private $codec;

    /**
     * @param AIngleIO $io
     * @param AIngleIODatumWriter $datum_writer
     * @param AIngleSchema $writers_schema
     * @param string $codec
     */
    public function __construct($io, $datum_writer, $writers_schema = null, $codec = AIngleDataIO::NULL_CODEC)
    {
        if (!($io instanceof AIngleIO)) {
            throw new AIngleDataIOException('io must be instance of AIngleIO');
        }

        $this->io = $io;
        $this->encoder = new AIngleIOBinaryEncoder($this->io);
        $this->datum_writer = $datum_writer;
        $this->buffer = new AIngleStringIO();
        $this->buffer_encoder = new AIngleIOBinaryEncoder($this->buffer);
        $this->block_count = 0;
        $this->metadata = array();

        if ($writers_schema) {
            if (!AIngleDataIO::isValidCodec($codec)) {
                throw new AIngleDataIOException(
                    sprintf('codec %s is not supported', $codec)
                );
            }

            $this->sync_marker = self::generateSyncMarker();
            $this->metadata[AIngleDataIO::METADATA_CODEC_ATTR] = $this->codec = $codec;
            $this->metadata[AIngleDataIO::METADATA_SCHEMA_ATTR] = (string) $writers_schema;
            $this->writeHeader();
        } else {
            $dfr = new AIngleDataIOReader($this->io, new AIngleIODatumReader());
            $this->sync_marker = $dfr->sync_marker;
            $this->metadata[AIngleDataIO::METADATA_CODEC_ATTR] = $this->codec
                = $dfr->metadata[AIngleDataIO::METADATA_CODEC_ATTR];
            $schema_from_file = $dfr->metadata[AIngleDataIO::METADATA_SCHEMA_ATTR];
            $this->metadata[AIngleDataIO::METADATA_SCHEMA_ATTR] = $schema_from_file;
            $this->datum_writer->writersSchema = AIngleSchema::parse($schema_from_file);
            $this->seek(0, SEEK_END);
        }
    }

    /**
     * @returns string a new, unique sync marker.
     */
    private static function generateSyncMarker()
    {
        // From https://php.net/manual/en/function.mt-rand.php comments
        return pack(
            'S8',
            random_int(0, 0xffff),
            random_int(0, 0xffff),
            random_int(0, 0xffff),
            random_int(0, 0xffff) | 0x4000,
            random_int(0, 0xffff) | 0x8000,
            random_int(0, 0xffff),
            random_int(0, 0xffff),
            random_int(0, 0xffff)
        );
    }

    /**
     * Writes the header of the AIngleIO object container
     */
    private function writeHeader()
    {
        $this->write(AIngleDataIO::magic());
        $this->datum_writer->writeData(
            AIngleDataIO::metadataSchema(),
            $this->metadata,
            $this->encoder
        );
        $this->write($this->sync_marker);
    }

    /**
     * @param string $bytes
     * @uses AIngleIO::write()
     */
    private function write($bytes)
    {
        return $this->io->write($bytes);
    }

    /**
     * @param int $offset
     * @param int $whence
     * @uses AIngleIO::seek()
     */
    private function seek($offset, $whence)
    {
        return $this->io->seek($offset, $whence);
    }

    /**
     * @param mixed $datum
     */
    public function append($datum)
    {
        $this->datum_writer->write($datum, $this->buffer_encoder);
        $this->block_count++;

        if ($this->buffer->length() >= AIngleDataIO::SYNC_INTERVAL) {
            $this->writeBlock();
        }
    }

    /**
     * Writes a block of data to the AIngleIO object container.
     */
    private function writeBlock()
    {
        if ($this->block_count > 0) {
            $this->encoder->writeLong($this->block_count);
            $to_write = (string) $this->buffer;

            if ($this->codec === AIngleDataIO::DEFLATE_CODEC) {
                $to_write = gzdeflate($to_write);
            } elseif ($this->codec === AIngleDataIO::ZSTANDARD_CODEC) {
                if (!extension_loaded('zstd')) {
                    throw new AIngleException('Please install ext-zstd to use zstandard compression.');
                }
                $to_write = zstd_compress($to_write);
            } elseif ($this->codec === AIngleDataIO::SNAPPY_CODEC) {
                if (!extension_loaded('snappy')) {
                    throw new AIngleException('Please install ext-snappy to use snappy compression.');
                }
                $crc32 = crc32($to_write);
                $compressed = snappy_compress($to_write);
                $to_write = pack('a*N', $compressed, $crc32);
            } elseif ($this->codec === AIngleDataIO::BZIP2_CODEC) {
                if (!extension_loaded('bz2')) {
                    throw new AIngleException('Please install ext-bz2 to use bzip2 compression.');
                }
                $to_write = bzcompress($to_write);
            }

            $this->encoder->writeLong(strlen($to_write));
            $this->write($to_write);
            $this->write($this->sync_marker);
            $this->buffer->truncate();
            $this->block_count = 0;
        }
    }

    /**
     * Flushes buffer to AIngleIO object container and closes it.
     * @return mixed value of $io->close()
     * @see AIngleIO::close()
     */
    public function close()
    {
        $this->flush();
        return $this->io->close();
    }

    /**
     * Flushes biffer to AIngleIO object container.
     * @returns mixed value of $io->flush()
     * @see AIngleIO::flush()
     */
    private function flush()
    {
        $this->writeBlock();
        return $this->io->flush();
    }
}
