<?php

/**

 */

namespace Apache\AIngle\Schema;

/**
 * AIngle schema for basic types such as null, int, long, string.
 * @package AIngle
 */
class AInglePrimitiveSchema extends AIngleSchema
{
    /**
     * @param string $type the primitive schema type name
     * @throws AIngleSchemaParseException if the given $type is not a
     *         primitive schema type name
     */
    public function __construct($type)
    {
        if (!self::isPrimitiveType($type)) {
            throw new AIngleSchemaParseException(sprintf('%s is not a valid primitive type.', $type));
        }
        parent::__construct($type);
    }

    /**
     * @returns mixed
     */
    public function toAIngle()
    {
        $aingle = parent::toAIngle();
        // FIXME: Is this if really necessary? When *wouldn't* this be the case?
        if (1 == count($aingle)) {
            return $this->type;
        }
        return $aingle;
    }
}
