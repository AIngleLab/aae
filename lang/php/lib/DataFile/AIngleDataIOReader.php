<?php

/**

 */

namespace Apache\AIngle\DataFile;

use Apache\AIngle\AIngleException;
use Apache\AIngle\AIngleIO;
use Apache\AIngle\AIngleUtil;
use Apache\AIngle\Datum\AIngleIOBinaryDecoder;
use Apache\AIngle\Datum\AIngleIODatumReader;
use Apache\AIngle\IO\AIngleStringIO;
use Apache\AIngle\Schema\AIngleSchema;

/**
 *
 * Reads AIngle data from an AIngleIO source using an AIngleSchema.
 * @package AIngle
 */
class AIngleDataIOReader
{
    /**
     * @var string
     */
    public $sync_marker;
    /**
     * @var array object container metadata
     */
    public $metadata;
    /**
     * @var AIngleIO
     */
    private $io;
    /**
     * @var AIngleIOBinaryDecoder
     */
    private $decoder;
    /**
     * @var AIngleIODatumReader
     */
    private $datum_reader;
    /**
     * @var int count of items in block
     */
    private $block_count;

    /**
     * @var compression codec
     */
    private $codec;

    /**
     * @param AIngleIO $io source from which to read
     * @param AIngleIODatumReader $datum_reader reader that understands
     *                                        the data schema
     * @throws AIngleDataIOException if $io is not an instance of AIngleIO
     *                             or the codec specified in the header
     *                             is not supported
     * @uses readHeader()
     */
    public function __construct($io, $datum_reader)
    {

        if (!($io instanceof AIngleIO)) {
            throw new AIngleDataIOException('io must be instance of AIngleIO');
        }

        $this->io = $io;
        $this->decoder = new AIngleIOBinaryDecoder($this->io);
        $this->datum_reader = $datum_reader;
        $this->readHeader();

        $codec = $this->metadata[AIngleDataIO::METADATA_CODEC_ATTR] ?? null;
        if ($codec && !AIngleDataIO::isValidCodec($codec)) {
            throw new AIngleDataIOException(sprintf('Unknown codec: %s', $codec));
        }
        $this->codec = $codec;

        $this->block_count = 0;
        // FIXME: Seems unsanitary to set writers_schema here.
        // Can't constructor take it as an argument?
        $this->datum_reader->setWritersSchema(
            AIngleSchema::parse($this->metadata[AIngleDataIO::METADATA_SCHEMA_ATTR])
        );
    }

    /**
     * Reads header of object container
     * @throws AIngleDataIOException if the file is not an AIngle data file.
     */
    private function readHeader()
    {
        $this->seek(0, AIngleIO::SEEK_SET);

        $magic = $this->read(AIngleDataIO::magicSize());

        if (strlen($magic) < AIngleDataIO::magicSize()) {
            throw new AIngleDataIOException(
                'Not an AIngle data file: shorter than the AIngle magic block'
            );
        }

        if (AIngleDataIO::magic() != $magic) {
            throw new AIngleDataIOException(
                sprintf(
                    'Not an AIngle data file: %s does not match %s',
                    $magic,
                    AIngleDataIO::magic()
                )
            );
        }

        $this->metadata = $this->datum_reader->readData(
            AIngleDataIO::metadataSchema(),
            AIngleDataIO::metadataSchema(),
            $this->decoder
        );
        $this->sync_marker = $this->read(AIngleDataIO::SYNC_SIZE);
    }

    /**
     * @uses AIngleIO::seek()
     */
    private function seek($offset, $whence)
    {
        return $this->io->seek($offset, $whence);
    }

    /**
     * @uses AIngleIO::read()
     */
    private function read($len)
    {
        return $this->io->read($len);
    }

    /**
     * @internal Would be nice to implement data() as an iterator, I think
     * @returns array of data from object container.
     */
    public function data()
    {
        $data = [];
        $decoder = $this->decoder;
        while (true) {
            if (0 == $this->block_count) {
                if ($this->isEof()) {
                    break;
                }

                if ($this->skipSync()) {
                    if ($this->isEof()) {
                        break;
                    }
                }

                $length = $this->readBlockHeader();
                if ($this->codec == AIngleDataIO::DEFLATE_CODEC) {
                    $compressed = $decoder->read($length);
                    $datum = gzinflate($compressed);
                    $decoder = new AIngleIOBinaryDecoder(new AIngleStringIO($datum));
                } elseif ($this->codec === AIngleDataIO::ZSTANDARD_CODEC) {
                    if (!extension_loaded('zstd')) {
                        throw new AIngleException('Please install ext-zstd to use zstandard compression.');
                    }
                    $compressed = $decoder->read($length);
                    $datum = zstd_uncompress($compressed);
                    $decoder = new AIngleIOBinaryDecoder(new AIngleStringIO($datum));
                } elseif ($this->codec === AIngleDataIO::SNAPPY_CODEC) {
                    if (!extension_loaded('snappy')) {
                        throw new AIngleException('Please install ext-snappy to use snappy compression.');
                    }
                    $compressed = $decoder->read($length);
                    $crc32 = unpack('N', substr($compressed, -4))[1];
                    $datum = snappy_uncompress(substr($compressed, 0, -4));
                    if ($crc32 === crc32($datum)) {
                        $decoder = new AIngleIOBinaryDecoder(new AIngleStringIO($datum));
                    } else {
                        $decoder = new AIngleIOBinaryDecoder(new AIngleStringIO(snappy_uncompress($datum)));
                    }
                } elseif ($this->codec === AIngleDataIO::BZIP2_CODEC) {
                    if (!extension_loaded('bz2')) {
                        throw new AIngleException('Please install ext-bz2 to use bzip2 compression.');
                    }
                    $compressed = $decoder->read($length);
                    $datum = bzdecompress($compressed);
                    $decoder = new AIngleIOBinaryDecoder(new AIngleStringIO($datum));
                }
            }
            $data[] = $this->datum_reader->read($decoder);
            --$this->block_count;
        }
        return $data;
    }

    /**
     * @uses AIngleIO::isEof()
     */
    private function isEof()
    {
        return $this->io->isEof();
    }

    private function skipSync()
    {
        $proposed_sync_marker = $this->read(AIngleDataIO::SYNC_SIZE);
        if ($proposed_sync_marker != $this->sync_marker) {
            $this->seek(-AIngleDataIO::SYNC_SIZE, AIngleIO::SEEK_CUR);
            return false;
        }
        return true;
    }

    /**
     * Reads the block header (which includes the count of items in the block
     * and the length in bytes of the block)
     * @returns int length in bytes of the block.
     */
    private function readBlockHeader()
    {
        $this->block_count = $this->decoder->readLong();
        return $this->decoder->readLong();
    }

    /**
     * Closes this writer (and its AIngleIO object.)
     * @uses AIngleIO::close()
     */
    public function close()
    {
        return $this->io->close();
    }
}
