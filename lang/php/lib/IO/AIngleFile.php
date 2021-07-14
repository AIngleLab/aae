<?php

/**

 */

namespace Apache\AIngle\IO;

use Apache\AIngle\AIngleIO;

/**
 * AIngleIO wrapper for PHP file access functions
 * @package AIngle
 */
class AIngleFile extends AIngleIO
{
    /**
     * @var string fopen read mode value. Used internally.
     */
    public const FOPEN_READ_MODE = 'rb';

    /**
     * @var string fopen write mode value. Used internally.
     */
    public const FOPEN_WRITE_MODE = 'wb';

    /**
     * @var string
     */
    private $file_path;

    /**
     * @var resource file handle for AIngleFile instance
     */
    private $file_handle;

    public function __construct($file_path, $mode = self::READ_MODE)
    {
        /**
         * XXX: should we check for file existence (in case of reading)
         * or anything else about the provided file_path argument?
         */
        $this->file_path = $file_path;
        switch ($mode) {
            case self::WRITE_MODE:
                $this->file_handle = fopen($this->file_path, self::FOPEN_WRITE_MODE);
                if (false == $this->file_handle) {
                    throw new AIngleIOException('Could not open file for writing');
                }
                break;
            case self::READ_MODE:
                $this->file_handle = fopen($this->file_path, self::FOPEN_READ_MODE);
                if (false == $this->file_handle) {
                    throw new AIngleIOException('Could not open file for reading');
                }
                break;
            default:
                throw new AIngleIOException(
                    sprintf(
                        "Only modes '%s' and '%s' allowed. You provided '%s'.",
                        self::READ_MODE,
                        self::WRITE_MODE,
                        $mode
                    )
                );
        }
    }

    /**
     * @returns int count of bytes written
     * @throws AIngleIOException if write failed.
     */
    public function write($str)
    {
        $len = fwrite($this->file_handle, $str);
        if (false === $len) {
            throw new AIngleIOException(sprintf('Could not write to file'));
        }
        return $len;
    }

    /**
     * @returns int current position within the file
     * @throws AIngleIOException if tell failed.
     */
    public function tell()
    {
        $position = ftell($this->file_handle);
        if (false === $position) {
            throw new AIngleIOException('Could not execute tell on reader');
        }
        return $position;
    }

    /**
     * Closes the file.
     * @returns boolean true if successful.
     * @throws AIngleIOException if there was an error closing the file.
     */
    public function close()
    {
        $res = fclose($this->file_handle);
        if (false === $res) {
            throw new AIngleIOException('Error closing file.');
        }
        return $res;
    }

    /**
     * @returns boolean true if the pointer is at the end of the file,
     *                  and false otherwise.
     * @see AIngleIO::isEof() as behavior differs from feof()
     */
    public function isEof()
    {
        $this->read(1);
        if (feof($this->file_handle)) {
            return true;
        }
        $this->seek(-1, self::SEEK_CUR);
        return false;
    }

    /**
     * @param int $len count of bytes to read.
     * @returns string bytes read
     * @throws AIngleIOException if length value is negative or if the read failed
     */
    public function read($len)
    {
        if (0 > $len) {
            throw new AIngleIOException(
                sprintf("Invalid length value passed to read: %d", $len)
            );
        }

        if (0 == $len) {
            return '';
        }

        $bytes = fread($this->file_handle, $len);
        if (false === $bytes) {
            throw new AIngleIOException('Could not read from file');
        }
        return $bytes;
    }

    /**
     * @param int $offset
     * @param int $whence
     * @returns boolean true upon success
     * @throws AIngleIOException if seek failed.
     * @see AIngleIO::seek()
     */
    public function seek($offset, $whence = SEEK_SET): bool
    {
        $res = fseek($this->file_handle, $offset, $whence);
        // Note: does not catch seeking beyond end of file
        if (-1 === $res) {
            throw new AIngleIOException(
                sprintf(
                    "Could not execute seek (offset = %d, whence = %d)",
                    $offset,
                    $whence
                )
            );
        }
        return true;
    }

    /**
     * @returns boolean true if the flush was successful.
     * @throws AIngleIOException if there was an error flushing the file.
     */
    public function flush()
    {
        $res = fflush($this->file_handle);
        if (false === $res) {
            throw new AIngleIOException('Could not flush file.');
        }
        return true;
    }
}
