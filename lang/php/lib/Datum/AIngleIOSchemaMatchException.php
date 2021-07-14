<?php

/**

 */

namespace Apache\AIngle\Datum;

use Apache\AIngle\AIngleException;
use Apache\AIngle\Schema\AIngleSchema;

/**
 * Exceptions arising from incompatibility between
 * reader and writer schemas.
 *
 * @package AIngle
 */
class AIngleIOSchemaMatchException extends AIngleException
{
    /**
     * @param AIngleSchema $writers_schema
     * @param AIngleSchema $readers_schema
     */
    public function __construct($writers_schema, $readers_schema)
    {
        parent::__construct(
            sprintf(
                "Writer's schema %s and Reader's schema %s do not match.",
                $writers_schema,
                $readers_schema
            )
        );
    }
}
