<?php

/**

 */

namespace Apache\AIngle\IO;

use Apache\AIngle\AIngleIO;

/**
 * AIngleIO wrapper for string access
 * @package AIngle
 */
class AIngleStringIO extends AIngleIO
{
    /**
     * @var string
     */
    private $string_buffer;
    /**
     * @var int  current position in string
     */
    private $current_index;
    /**
     * @var boolean whether or not the string is closed.
     */
    private $is_closed;

    /**
     * @param string $str initial value of AIngleStringIO buffer. Regardless
     *                    of the initial value, the pointer is set to the
     *                    beginning of the buffer.
     * @throws AIngleIOException if a non-string value is passed as $str
     */
    public function __construct($str = '')
    {
        $this->is_closed = false;
        $this->string_buffer = '';
        $this->current_index = 0;

        if (is_string($str)) {
            $this->string_buffer .= $str;
        } else {
            throw new AIngleIOException(
                sprintf('constructor argument must be a string: %s', gettype($str))
            );
        }
    }

    /**
     * Append bytes to this buffer.
     * (Nothing more is needed to support AIngle.)
     * @param string $arg bytes to write
     * @returns int count of bytes written.
     * @throws AIngleIOException if $args is not a string value.
     */
    public function write($arg)
    {
        $this->checkClosed();
        if (is_string($arg)) {
            return $this->appendStr($arg);
        }
        throw new AIngleIOException(
            sprintf(
                'write argument must be a string: (%s) %s',
                gettype($arg),
                var_export($arg, true)
            )
        );
    }

    /**
     * @throws AIngleIOException if the buffer is closed.
     */
    private function checkClosed()
    {
        if ($this->isClosed()) {
            throw new AIngleIOException('Buffer is closed');
        }
    }

    /**
     * @returns boolean true if this buffer is closed and false
     *                       otherwise.
     */
    public function isClosed()
    {
        return $this->is_closed;
    }

    /**
     * Appends bytes to this buffer.
     * @param string $str
     * @returns integer count of bytes written.
     */
    private function appendStr($str)
    {
        $this->checkClosed();
        $this->string_buffer .= $str;
        $len = strlen($str);
        $this->current_index += $len;
        return $len;
    }

    /**
     * @returns string bytes read from buffer
     * @todo test for fencepost errors wrt updating current_index
     */
    public function read($len)
    {
        $this->checkClosed();
        $read = '';
        for ($i = $this->current_index; $i < ($this->current_index + $len); $i++) {
            $read .= $this->string_buffer[$i] ?? '';
        }
        if (strlen($read) < $len) {
            $this->current_index = $this->length();
        } else {
            $this->current_index += $len;
        }
        return $read;
    }

    /**
     * @returns int count of bytes in the buffer
     * @internal Could probably memoize length for performance, but
     *           no need do this yet.
     */
    public function length()
    {
        return strlen($this->string_buffer);
    }

    /**
     * @returns boolean true if successful
     * @throws AIngleIOException if the seek failed.
     */
    public function seek($offset, $whence = self::SEEK_SET): bool
    {
        if (!is_int($offset)) {
            throw new AIngleIOException('Seek offset must be an integer.');
        }
        // Prevent seeking before BOF
        switch ($whence) {
            case self::SEEK_SET:
                if (0 > $offset) {
                    throw new AIngleIOException('Cannot seek before beginning of file.');
                }
                $this->current_index = $offset;
                break;
            case self::SEEK_CUR:
                if (0 > $this->current_index + $whence) {
                    throw new AIngleIOException('Cannot seek before beginning of file.');
                }
                $this->current_index += $offset;
                break;
            case self::SEEK_END:
                if (0 > $this->length() + $offset) {
                    throw new AIngleIOException('Cannot seek before beginning of file.');
                }
                $this->current_index = $this->length() + $offset;
                break;
            default:
                throw new AIngleIOException(sprintf('Invalid seek whence %d', $whence));
        }

        return true;
    }

    /**
     * @returns int
     * @see AIngleIO::tell()
     */
    public function tell()
    {
        return $this->current_index;
    }

    /**
     * @returns boolean
     * @see AIngleIO::isEof()
     */
    public function isEof()
    {
        return ($this->current_index >= $this->length());
    }

    /**
     * No-op provided for compatibility with AIngleIO interface.
     * @returns boolean true
     */
    public function flush()
    {
        return true;
    }

    /**
     * Marks this buffer as closed.
     * @returns boolean true
     */
    public function close()
    {
        $this->checkClosed();
        $this->is_closed = true;
        return true;
    }

    /**
     * Truncates the truncate buffer to 0 bytes and returns the pointer
     * to the beginning of the buffer.
     * @returns boolean true
     */
    public function truncate()
    {
        $this->checkClosed();
        $this->string_buffer = '';
        $this->current_index = 0;
        return true;
    }

    /**
     * @returns string
     * @uses self::__toString()
     */
    public function string()
    {
        return (string) $this;
    }

    /**
     * @returns string
     */
    public function __toString()
    {
        return $this->string_buffer;
    }
}
