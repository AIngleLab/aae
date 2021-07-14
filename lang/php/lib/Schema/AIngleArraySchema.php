<?php

/**

 */

namespace Apache\AIngle\Schema;

/**
 * AIngle array schema, consisting of items of a particular
 * AIngle schema type.
 * @package AIngle
 */
class AIngleArraySchema extends AIngleSchema
{
    /**
     * @var AIngleName|AIngleSchema named schema name or AIngleSchema of
     *                          array element
     */
    private $items;

    /**
     * @var boolean true if the items schema
     * FIXME: couldn't we derive this from whether or not $this->items
     *        is an AIngleName or an AIngleSchema?
     */
    private $is_items_schema_from_schemata;

    /**
     * @param string|mixed $items AIngleNamedSchema name or object form
     *        of decoded JSON schema representation.
     * @param string $defaultNamespace namespace of enclosing schema
     * @param AIngleNamedSchemata &$schemata
     */
    public function __construct($items, $defaultNamespace, &$schemata = null)
    {
        parent::__construct(AIngleSchema::ARRAY_SCHEMA);

        $this->is_items_schema_from_schemata = false;
        $items_schema = null;
        if (
            is_string($items)
            && $items_schema = $schemata->schemaByName(
                new AIngleName($items, null, $defaultNamespace)
            )
        ) {
            $this->is_items_schema_from_schemata = true;
        } else {
            $items_schema = AIngleSchema::subparse($items, $defaultNamespace, $schemata);
        }

        $this->items = $items_schema;
    }

    /**
     * @returns AIngleName|AIngleSchema named schema name or AIngleSchema
     *          of this array schema's elements.
     */
    public function items()
    {
        return $this->items;
    }

    /**
     * @returns mixed
     */
    public function toAIngle()
    {
        $aingle = parent::toAIngle();
        $aingle[AIngleSchema::ITEMS_ATTR] = $this->is_items_schema_from_schemata
            ? $this->items->qualifiedName() : $this->items->toAIngle();
        return $aingle;
    }
}
