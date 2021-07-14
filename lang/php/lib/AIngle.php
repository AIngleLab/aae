<?php

/**

 */

namespace Apache\AIngle;

/**
 * Library-level class for PHP AIngle port.
 *
 * Contains library details such as version number and platform checks.
 *
 * This port is an implementation of the
 * {@link https://apache.aingle.ai/docs/1.3.3/spec.html AIngle 1.3.3 Specification}
 *
 * @package AIngle
 */
class AIngle
{
    /**
     * @var string version number of AIngle specification to which
     *             this implemenation complies
     */
    public const SPEC_VERSION = '1.4.0';

    /**#@+
     * Constant to enumerate biginteger handling mode.
     * GMP is used, if available, on 32-bit platforms.
     */
    private const PHP_BIGINTEGER_MODE = 0x00;
    private const GMP_BIGINTEGER_MODE = 0x01;
    /**#@-*/
    /**
     * @var int
     * Mode used to handle bigintegers. After AIngle::check64Bit() has been called,
     * (usually via a call to AIngle::checkPlatform(), set to
     * self::GMP_BIGINTEGER_MODE on 32-bit platforms that have GMP available,
     * and to self::PHP_BIGINTEGER_MODE otherwise.
     */
    private static $biginteger_mode;

    /**
     * Wrapper method to call each required check.
     *
     */
    public static function checkPlatform()
    {
        self::check64Bit();
    }

    /**
     * Determines if the host platform can encode and decode long integer data.
     *
     * @throws AIngleException if the platform cannot handle long integers.
     */
    private static function check64Bit()
    {
        if (8 != PHP_INT_SIZE) {
            if (extension_loaded('gmp')) {
                self::$biginteger_mode = self::GMP_BIGINTEGER_MODE;
            } else {
                throw new AIngleException('This platform cannot handle a 64-bit operations. '
                    . 'Please install the GMP PHP extension.');
            }
        } else {
            self::$biginteger_mode = self::PHP_BIGINTEGER_MODE;
        }
    }

    /**
     * @returns boolean true if the PHP GMP extension is used and false otherwise.
     * @internal Requires AIngle::check64Bit() (exposed via AIngle::checkPlatform())
     *           to have been called to set AIngle::$biginteger_mode.
     */
    public static function usesGmp()
    {
        return self::GMP_BIGINTEGER_MODE === self::$biginteger_mode;
    }
}
