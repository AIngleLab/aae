<?php

/**

 */

namespace Apache\AIngle\Datum;

use Apache\AIngle\AIngleException;
use Apache\AIngle\Schema\AIngleSchema;

/**
 * Exceptions arising from writing or reading AIngle data.
 *
 * @package AIngle
 */
class AIngleIOTypeException extends AIngleException
{
    /**
     * @param AIngleSchema $expectedSchema
     * @param mixed $datum
     */
    public function __construct($expectedSchema, $datum)
    {
        parent::__construct(sprintf(
            'The datum %s is not an example of schema %s',
            var_export($datum, true),
            $expectedSchema
        ));
    }
}
