<?php

/**

 */

namespace Apache\AIngle\Schema;

/**
 * AIngle map schema consisting of named values of defined
 * AIngle Schema types.
 * @package AIngle
 */
class AIngleMapSchema extends AIngleSchema
{
    /**
     * @var string|AIngleSchema named schema name or AIngleSchema
     *      of map schema values.
     */
    private $values;

    /**
     * @var boolean true if the named schema
     * XXX Couldn't we derive this based on whether or not
     * $this->values is a string?
     */
    private $isValuesSchemaFromSchemata;

    /**
     * @param string|AIngleSchema $values
     * @param string $defaultNamespace namespace of enclosing schema
     * @param AIngleNamedSchemata &$schemata
     */
    public function __construct($values, $defaultNamespace, &$schemata = null)
    {
        parent::__construct(AIngleSchema::MAP_SCHEMA);

        $this->isValuesSchemaFromSchemata = false;
        $values_schema = null;
        if (
            is_string($values)
            && $values_schema = $schemata->schemaByName(
                new AIngleName($values, null, $defaultNamespace)
            )
        ) {
            $this->isValuesSchemaFromSchemata = true;
        } else {
            $values_schema = AIngleSchema::subparse(
                $values,
                $defaultNamespace,
                $schemata
            );
        }

        $this->values = $values_schema;
    }

    /**
     * @returns XXX|AIngleSchema
     */
    public function values()
    {
        return $this->values;
    }

    /**
     * @returns mixed
     */
    public function toAIngle()
    {
        $aingle = parent::toAIngle();
        $aingle[AIngleSchema::VALUES_ATTR] = $this->isValuesSchemaFromSchemata
            ? $this->values->qualifiedName() : $this->values->toAIngle();
        return $aingle;
    }
}
