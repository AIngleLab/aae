<?php

/**

 */

namespace Apache\AIngle\Datum;

// @todo Implement JSON encoding, as is required by the AIngle spec.
use Apache\AIngle\AIngle;
use Apache\AIngle\AIngleException;
use Apache\AIngle\AIngleGMP;
use Apache\AIngle\AIngleIO;

/**
 * Decodes and reads AIngle data from an AIngleIO object encoded using
 * AIngle binary encoding.
 *
 * @package AIngle
 */
class AIngleIOBinaryDecoder
{

    /**
     * @var AIngleIO
     */
    private $io;

    /**
     * @param AIngleIO $io object from which to read.
     */
    public function __construct($io)
    {
        AIngle::checkPlatform();
        $this->io = $io;
    }

    /**
     * @returns null
     */
    public function readNull()
    {
        return null;
    }

    /**
     * @returns boolean
     */
    public function readBoolean()
    {
        return (bool) (1 == ord($this->nextByte()));
    }

    /**
     * @returns string the next byte from $this->io.
     * @throws AIngleException if the next byte cannot be read.
     */
    private function nextByte()
    {
        return $this->read(1);
    }

    /**
     * @param int $len count of bytes to read
     * @returns string
     */
    public function read($len)
    {
        return $this->io->read($len);
    }

    /**
     * @returns int
     */
    public function readInt()
    {
        return (int) $this->readLong();
    }

    /**
     * @returns string|int
     */
    public function readLong()
    {
        $byte = ord($this->nextByte());
        $bytes = array($byte);
        while (0 != ($byte & 0x80)) {
            $byte = ord($this->nextByte());
            $bytes [] = $byte;
        }

        if (AIngle::usesGmp()) {
            return AIngleGMP::decodeLongFromArray($bytes);
        }

        return self::decodeLongFromArray($bytes);
    }

    /**
     * @param int[] array of byte ascii values
     * @returns long decoded value
     * @internal Requires 64-bit platform
     */
    public static function decodeLongFromArray($bytes)
    {
        $b = array_shift($bytes);
        $n = $b & 0x7f;
        $shift = 7;
        while (0 != ($b & 0x80)) {
            $b = array_shift($bytes);
            $n |= (($b & 0x7f) << $shift);
            $shift += 7;
        }
        return (($n >> 1) ^ -($n & 1));
    }

    /**
     * @returns float
     */
    public function readFloat()
    {
        return self::intBitsToFloat($this->read(4));
    }

    /**
     * Performs decoding of the binary string to a float value.
     *
     * XXX: This is <b>not</b> endian-aware! See comments in
     * {@link AIngleIOBinaryEncoder::floatToIntBits()} for details.
     *
     * @param string $bits
     * @returns float
     */
    public static function intBitsToFloat($bits)
    {
        $float = unpack('g', $bits);
        return (float) $float[1];
    }

    /**
     * @returns double
     */
    public function readDouble()
    {
        return self::longBitsToDouble($this->read(8));
    }

    /**
     * Performs decoding of the binary string to a double value.
     *
     * XXX: This is <b>not</b> endian-aware! See comments in
     * {@link AIngleIOBinaryEncoder::floatToIntBits()} for details.
     *
     * @param string $bits
     * @returns float
     */
    public static function longBitsToDouble($bits)
    {
        $double = unpack('e', $bits);
        return (double) $double[1];
    }

    /**
     * A string is encoded as a long followed by that many bytes
     * of UTF-8 encoded character data.
     * @returns string
     */
    public function readString()
    {
        return $this->readBytes();
    }

    /**
     * @returns string
     */
    public function readBytes()
    {
        return $this->read($this->readLong());
    }

    public function skipNull()
    {
        return null;
    }

    public function skipBoolean()
    {
        return $this->skip(1);
    }

    /**
     * @param int $len count of bytes to skip
     * @uses AIngleIO::seek()
     */
    public function skip($len)
    {
        $this->seek($len, AIngleIO::SEEK_CUR);
    }

    /**
     * @param int $offset
     * @param int $whence
     * @returns boolean true upon success
     * @uses AIngleIO::seek()
     */
    private function seek($offset, $whence)
    {
        return $this->io->seek($offset, $whence);
    }

    public function skipInt()
    {
        return $this->skipLong();
    }

    public function skipLong()
    {
        $b = ord($this->nextByte());
        while (0 != ($b & 0x80)) {
            $b = ord($this->nextByte());
        }
    }

    public function skipFloat()
    {
        return $this->skip(4);
    }

    public function skipDouble()
    {
        return $this->skip(8);
    }

    public function skipString()
    {
        return $this->skipBytes();
    }

    public function skipBytes()
    {
        return $this->skip($this->readLong());
    }

    public function skipFixed($writers_schema, AIngleIOBinaryDecoder $decoder)
    {
        $decoder->skip($writers_schema->size());
    }

    public function skipEnum($writers_schema, AIngleIOBinaryDecoder $decoder)
    {
        $decoder->skipInt();
    }

    public function skipUnion($writers_schema, AIngleIOBinaryDecoder $decoder)
    {
        $index = $decoder->readLong();
        AIngleIODatumReader::skipData($writers_schema->schemaByIndex($index), $decoder);
    }

    public function skipRecord($writers_schema, AIngleIOBinaryDecoder $decoder)
    {
        foreach ($writers_schema->fields() as $f) {
            AIngleIODatumReader::skipData($f->type(), $decoder);
        }
    }

    public function skipArray($writers_schema, AIngleIOBinaryDecoder $decoder)
    {
        $block_count = $decoder->readLong();
        while (0 !== $block_count) {
            if ($block_count < 0) {
                $decoder->skip($this->readLong());
            }
            for ($i = 0; $i < $block_count; $i++) {
                AIngleIODatumReader::skipData($writers_schema->items(), $decoder);
            }
            $block_count = $decoder->readLong();
        }
    }

    public function skipMap($writers_schema, AIngleIOBinaryDecoder $decoder)
    {
        $block_count = $decoder->readLong();
        while (0 !== $block_count) {
            if ($block_count < 0) {
                $decoder->skip($this->readLong());
            }
            for ($i = 0; $i < $block_count; $i++) {
                $decoder->skipString();
                AIngleIODatumReader::skipData($writers_schema->values(), $decoder);
            }
            $block_count = $decoder->readLong();
        }
    }

    /**
     * @returns int position of pointer in AIngleIO instance
     * @uses AIngleIO::tell()
     */
    private function tell()
    {
        return $this->io->tell();
    }
}
