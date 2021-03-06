<?php

/**

 */

namespace Apache\AIngle\Schema;

/**
 * Field of an {@link AIngleRecordSchema}
 * @package AIngle
 */
class AIngleField extends AIngleSchema
{
    /**
     * @var string fields name attribute name
     */
    public const FIELD_NAME_ATTR = 'name';

    /**
     * @var string
     */
    public const DEFAULT_ATTR = 'default';

    /**
     * @var string
     */
    public const ORDER_ATTR = 'order';

    /**
     * @var string
     */
    public const ASC_SORT_ORDER = 'ascending';

    /**
     * @var string
     */
    public const DESC_SORT_ORDER = 'descending';

    /**
     * @var string
     */
    public const IGNORE_SORT_ORDER = 'ignore';

    /**
     * @var array list of valid field sort order values
     */
    private static $validFieldSortOrders = array(
        self::ASC_SORT_ORDER,
        self::DESC_SORT_ORDER,
        self::IGNORE_SORT_ORDER
    );
    /**
     * @var string
     */
    private $name;
    /**
     * @var boolean whether or no there is a default value
     */
    private $hasDefault;
    /**
     * @var string field default value
     */
    private $default;
    /**
     * @var string sort order of this field
     */
    private $order;
    /**
     * @var boolean whether or not the AIngleNamedSchema of this field is
     *              defined in the AIngleNamedSchemata instance
     */
    private $isTypeFromSchemata;
    /**
     * @var array|null
     */
    private $aliases;

    /**
     * @param string $name
     * @param AIngleSchema $schema
     * @param boolean $is_type_from_schemata
     * @param $has_default
     * @param string $default
     * @param string $order
     * @param array $aliases
     * @throws AIngleSchemaParseException
     * @todo Check validity of $default value
     * @todo Check validity of $order value
     */
    public function __construct(
        $name,
        $schema,
        $is_type_from_schemata,
        $has_default,
        $default,
        $order = null,
        $aliases = null
    ) {
        if (!AIngleName::isWellFormedName($name)) {
            throw new AIngleSchemaParseException('Field requires a "name" attribute');
        }

        parent::__construct($schema);
        $this->isTypeFromSchemata = $is_type_from_schemata;
        $this->name = $name;
        $this->hasDefault = $has_default;
        if ($this->hasDefault) {
            $this->default = $default;
        }
        self::checkOrderValue($order);
        $this->order = $order;
        self::hasValidAliases($aliases);
        $this->aliases = $aliases;
    }

    /**
     * @param string $order
     * @throws AIngleSchemaParseException if $order is not a valid
     *                                  field order value.
     */
    private static function checkOrderValue($order)
    {
        if (!is_null($order) && !self::isValidFieldSortOrder($order)) {
            throw new AIngleSchemaParseException(
                sprintf('Invalid field sort order %s', $order)
            );
        }
    }

    /**
     * @param string $order
     * @returns boolean
     */
    private static function isValidFieldSortOrder($order)
    {
        return in_array($order, self::$validFieldSortOrders);
    }

    /**
     * @returns mixed
     */
    public function toAIngle()
    {
        $aingle = array(AIngleField::FIELD_NAME_ATTR => $this->name);

        $aingle[AIngleSchema::TYPE_ATTR] = ($this->isTypeFromSchemata)
            ? $this->type->qualifiedName() : $this->type->toAIngle();

        if (isset($this->default)) {
            $aingle[AIngleField::DEFAULT_ATTR] = $this->default;
        }

        if ($this->order) {
            $aingle[AIngleField::ORDER_ATTR] = $this->order;
        }

        return $aingle;
    }

    /**
     * @returns string the name of this field
     */
    public function name()
    {
        return $this->name;
    }

    /**
     * @returns mixed the default value of this field
     */
    public function defaultValue()
    {
        return $this->default;
    }

    /**
     * @returns boolean true if the field has a default and false otherwise
     */
    public function hasDefaultValue()
    {
        return $this->hasDefault;
    }

    public function getAliases()
    {
        return $this->aliases;
    }

    public function hasAliases()
    {
        return $this->aliases !== null;
    }
}
