<?php

/**

 */

namespace Apache\AIngle;

/**
 * Barebones IO base class to provide common interface for file and string
 * access within the AIngle classes.
 *
 * @package AIngle
 */
class AIngleIO
{
    /**
     * @var string general read mode
     */
    public const READ_MODE = 'r';
    /**
     * @var string general write mode.
     */
    public const WRITE_MODE = 'w';

    /**
     * @var int set position to current index + $offset bytes
     */
    public const SEEK_CUR = SEEK_CUR;
    /**
     * @var int set position equal to $offset bytes
     */
    public const SEEK_SET = SEEK_SET;
    /**
     * @var int set position to end of file + $offset bytes
     */
    public const SEEK_END = SEEK_END;

    /**
     * Read $len bytes from AIngleIO instance
     * @return string bytes read
     * @var int $len
     */
    public function read($len)
    {
        throw new AIngleNotImplementedException('Not implemented');
    }

    /**
     * Append bytes to this buffer. (Nothing more is needed to support AIngle.)
     * @param string $arg bytes to write
     * @returns int count of bytes written.
     * @throws IO\AIngleIOException if $args is not a string value.
     */
    public function write($arg)
    {
        throw new AIngleNotImplementedException('Not implemented');
    }

    /**
     * Return byte offset within AIngleIO instance
     * @return int
     */
    public function tell()
    {
        throw new AIngleNotImplementedException('Not implemented');
    }

    /**
     * Set the position indicator. The new position, measured in bytes
     * from the beginning of the file, is obtained by adding $offset to
     * the position specified by $whence.
     *
     * @param int $offset
     * @param int $whence one of AIngleIO::SEEK_SET, AIngleIO::SEEK_CUR,
     *                    or AIngle::SEEK_END
     * @returns boolean true
     *
     * @throws IO\AIngleIOException
     */
    public function seek($offset, $whence = self::SEEK_SET): bool
    {
        throw new AIngleNotImplementedException('Not implemented');
    }

    /**
     * Flushes any buffered data to the AIngleIO object.
     * @returns boolean true upon success.
     */
    public function flush()
    {
        throw new AIngleNotImplementedException('Not implemented');
    }

    /**
     * Returns whether or not the current position at the end of this AIngleIO
     * instance.
     *
     * Note isEof() is <b>not</b> like eof in C or feof in PHP:
     * it returns TRUE if the *next* read would be end of file,
     * rather than if the *most recent* read read end of file.
     * @returns boolean true if at the end of file, and false otherwise
     */
    public function isEof()
    {
        throw new AIngleNotImplementedException('Not implemented');
    }

    /**
     * Closes this AIngleIO instance.
     */
    public function close()
    {
        throw new AIngleNotImplementedException('Not implemented');
    }
}
