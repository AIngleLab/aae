<?php

/**

 */

namespace Apache\AIngle\Datum;

use Apache\AIngle\AIngle;
use Apache\AIngle\AIngleGMP;
use Apache\AIngle\AIngleIO;

/**
 * Encodes and writes AIngle data to an AIngleIO object using
 * AIngle binary encoding.
 *
 * @package AIngle
 */
class AIngleIOBinaryEncoder
{
    /**
     * @var AIngleIO
     */
    private $io;

    /**
     * @param AIngleIO $io object to which data is to be written.
     *
     */
    public function __construct($io)
    {
        AIngle::checkPlatform();
        $this->io = $io;
    }

    /**
     * @param null $datum actual value is ignored
     */
    public function writeNull($datum)
    {
        return null;
    }

    /**
     * @param boolean $datum
     */
    public function writeBoolean($datum)
    {
        $byte = $datum ? chr(1) : chr(0);
        $this->write($byte);
    }

    /**
     * @param string $datum
     */
    public function write($datum)
    {
        $this->io->write($datum);
    }

    /**
     * @param int $datum
     */
    public function writeInt($datum)
    {
        $this->writeLong($datum);
    }

    /**
     * @param int $n
     */
    public function writeLong($n)
    {
        if (AIngle::usesGmp()) {
            $this->write(AIngleGMP::encodeLong($n));
        } else {
            $this->write(self::encodeLong($n));
        }
    }

    /**
     * @param int|string $n
     * @returns string long $n encoded as bytes
     * @internal This relies on 64-bit PHP.
     */
    public static function encodeLong($n)
    {
        $n = (int) $n;
        $n = ($n << 1) ^ ($n >> 63);
        $str = '';
        while (0 != ($n & ~0x7F)) {
            $str .= chr(($n & 0x7F) | 0x80);
            $n >>= 7;
        }
        $str .= chr($n);
        return $str;
    }

    /**
     * @param float $datum
     * @uses self::floatToIntBits()
     */
    public function writeFloat($datum)
    {
        $this->write(self::floatToIntBits($datum));
    }

    /**
     * Performs encoding of the given float value to a binary string
     *
     * XXX: This is <b>not</b> endian-aware! The {@link AIngle::checkPlatform()}
     * called in {@link AIngleIOBinaryEncoder::__construct()} should ensure the
     * library is only used on little-endian platforms, which ensure the little-endian
     * encoding required by the AIngle spec.
     *
     * @param float $float
     * @returns string bytes
     * @see AIngle::checkPlatform()
     */
    public static function floatToIntBits($float)
    {
        return pack('g', (float) $float);
    }

    /**
     * @param float $datum
     * @uses self::doubleToLongBits()
     */
    public function writeDouble($datum)
    {
        $this->write(self::doubleToLongBits($datum));
    }

    /**
     * Performs encoding of the given double value to a binary string
     *
     * XXX: This is <b>not</b> endian-aware! See comments in
     * {@link AIngleIOBinaryEncoder::floatToIntBits()} for details.
     *
     * @param double $double
     * @returns string bytes
     */
    public static function doubleToLongBits($double)
    {
        return pack('e', (double) $double);
    }

    /**
     * @param string $str
     * @uses self::writeBytes()
     */
    public function writeString($str)
    {
        $this->writeBytes($str);
    }

    /**
     * @param string $bytes
     */
    public function writeBytes($bytes)
    {
        $this->writeLong(strlen($bytes));
        $this->write($bytes);
    }
}
