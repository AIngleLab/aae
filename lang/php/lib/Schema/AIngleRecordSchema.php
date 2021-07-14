<?php

/**

 */

namespace Apache\AIngle\Schema;

/**
 * @package AIngle
 */
class AIngleRecordSchema extends AIngleNamedSchema
{
    /**
     * @var AIngleNamedSchema[] array of AIngleNamedSchema field definitions of
     *                   this AIngleRecordSchema
     */
    private $fields;
    /**
     * @var array map of field names to field objects.
     * @internal Not called directly. Memoization of AIngleRecordSchema->fieldsHash()
     */
    private $fieldsHash;

    /**
     * @param AIngleName $name
     * @param string $namespace
     * @param string $doc
     * @param array $fields
     * @param AIngleNamedSchemata &$schemata
     * @param string $schema_type schema type name
     * @throws AIngleSchemaParseException
     */
    public function __construct(
        $name,
        $doc,
        $fields,
        &$schemata = null,
        $schema_type = AIngleSchema::RECORD_SCHEMA,
        $aliases = null
    ) {
        if (is_null($fields)) {
            throw new AIngleSchemaParseException(
                'Record schema requires a non-empty fields attribute'
            );
        }

        if (AIngleSchema::REQUEST_SCHEMA == $schema_type) {
            parent::__construct($schema_type, $name);
        } else {
            parent::__construct($schema_type, $name, $doc, $schemata, $aliases);
        }

        [$x, $namespace] = $name->nameAndNamespace();
        $this->fields = self::parseFields($fields, $namespace, $schemata);
    }

    /**
     * @param mixed $field_data
     * @param string $default_namespace namespace of enclosing schema
     * @param AIngleNamedSchemata &$schemata
     * @returns AIngleField[]
     * @throws AIngleSchemaParseException
     */
    public static function parseFields($field_data, $default_namespace, &$schemata)
    {
        $fields = array();
        $field_names = array();
        $alias_names = [];
        foreach ($field_data as $index => $field) {
            $name = $field[AIngleField::FIELD_NAME_ATTR] ?? null;
            $type = $field[AIngleSchema::TYPE_ATTR] ?? null;
            $order = $field[AIngleField::ORDER_ATTR] ?? null;
            $aliases = $field[AIngleField::ALIASES_ATTR] ?? null;

            $default = null;
            $has_default = false;
            if (array_key_exists(AIngleField::DEFAULT_ATTR, $field)) {
                $default = $field[AIngleField::DEFAULT_ATTR];
                $has_default = true;
            }

            if (in_array($name, $field_names)) {
                throw new AIngleSchemaParseException(
                    sprintf("Field name %s is already in use", $name)
                );
            }

            $is_schema_from_schemata = false;
            $field_schema = null;
            if (
                is_string($type)
                && $field_schema = $schemata->schemaByName(
                    new AIngleName($type, null, $default_namespace)
                )
            ) {
                $is_schema_from_schemata = true;
            } else {
                $field_schema = self::subparse($type, $default_namespace, $schemata);
            }

            $new_field = new AIngleField(
                $name,
                $field_schema,
                $is_schema_from_schemata,
                $has_default,
                $default,
                $order,
                $aliases
            );
            $field_names[] = $name;
            if ($new_field->hasAliases() && array_intersect($alias_names, $new_field->getAliases())) {
                throw new AIngleSchemaParseException("Alias already in use");
            }
            if ($new_field->hasAliases()) {
                array_push($alias_names, ...$new_field->getAliases());
            }
            $fields[] = $new_field;
        }
        return $fields;
    }

    /**
     * @returns mixed
     */
    public function toAIngle()
    {
        $aingle = parent::toAIngle();

        $fields_aingle = array();
        foreach ($this->fields as $field) {
            $fields_aingle[] = $field->toAIngle();
        }

        if (AIngleSchema::REQUEST_SCHEMA === $this->type) {
            return $fields_aingle;
        }

        $aingle[AIngleSchema::FIELDS_ATTR] = $fields_aingle;

        return $aingle;
    }

    /**
     * @returns array the schema definitions of the fields of this AIngleRecordSchema
     */
    public function fields()
    {
        return $this->fields;
    }

    /**
     * @returns array a hash table of the fields of this AIngleRecordSchema fields
     *          keyed by each field's name
     */
    public function fieldsHash()
    {
        if (is_null($this->fieldsHash)) {
            $hash = array();
            foreach ($this->fields as $field) {
                $hash[$field->name()] = $field;
            }
            $this->fieldsHash = $hash;
        }
        return $this->fieldsHash;
    }

    public function fieldsByAlias()
    {
        $hash = [];
        foreach ($this->fields as $field) {
            if ($field->hasAliases()) {
                foreach ($field->getAliases() as $a) {
                    $hash[$a] = $field;
                }
            }
        }
        return $hash;
    }
}
