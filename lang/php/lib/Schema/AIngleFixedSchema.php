<?php

/**

 */

namespace Apache\AIngle\Schema;

/**
 * AIngleNamedSchema with fixed-length data values
 * @package AIngle
 */
class AIngleFixedSchema extends AIngleNamedSchema
{
    /**
     * @var int byte count of this fixed schema data value
     */
    private $size;

    /**
     * @param AIngleName $name
     * @param string $doc Set to null, as fixed schemas don't have doc strings
     * @param int $size byte count of this fixed schema data value
     * @param AIngleNamedSchemata &$schemata
     * @param array $aliases
     * @throws AIngleSchemaParseException
     */
    public function __construct($name, $doc, $size, &$schemata = null, $aliases = null)
    {
        if (!is_int($size)) {
            throw new AIngleSchemaParseException(
                'Fixed Schema requires a valid integer for "size" attribute'
            );
        }
        parent::__construct(AIngleSchema::FIXED_SCHEMA, $name, null, $schemata, $aliases);
        $this->size = $size;
    }

    /**
     * @returns int byte count of this fixed schema data value
     */
    public function size()
    {
        return $this->size;
    }

    /**
     * @returns mixed
     */
    public function toAIngle()
    {
        $aingle = parent::toAIngle();
        $aingle[AIngleSchema::SIZE_ATTR] = $this->size;
        return $aingle;
    }
}
