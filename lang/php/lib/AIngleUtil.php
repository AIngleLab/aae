<?php

/**

 */

namespace Apache\AIngle;

/**
 * Class for static utility methods used in AIngle.
 *
 * @package AIngle
 */
class AIngleUtil
{
    /**
     * Determines whether the given array is an associative array
     * (what is termed a map, hash, or dictionary in other languages)
     * or a list (an array with monotonically increasing integer indicies
     * starting with zero).
     *
     * @param array $ary array to test
     * @returns true if the array is a list and false otherwise.
     *
     */
    public static function isList($ary): bool
    {
        if (is_array($ary)) {
            $i = 0;
            foreach ($ary as $k => $v) {
                if ($i !== $k) {
                    return false;
                }
                $i++;
            }
            return true;
        }
        return false;
    }
}
