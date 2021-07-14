<?php

/**

 */

namespace Apache\AIngle\Schema;

/**
 * Union of AIngle schemas, of which values can be of any of the schema in
 * the union.
 * @package AIngle
 */
class AIngleUnionSchema extends AIngleSchema
{
    /**
     * @var int[] list of indices of named schemas which
     *                are defined in $schemata
     */
    public $schemaFromSchemataIndices;
    /**
     * @var AIngleSchema[] list of schemas of this union
     */
    private $schemas;

    /**
     * @param AIngleSchema[] $schemas list of schemas in the union
     * @param string $defaultNamespace namespace of enclosing schema
     * @param AIngleNamedSchemata &$schemata
     */
    public function __construct($schemas, $defaultNamespace, &$schemata = null)
    {
        parent::__construct(AIngleSchema::UNION_SCHEMA);

        $this->schemaFromSchemataIndices = array();
        $schema_types = array();
        foreach ($schemas as $index => $schema) {
            $is_schema_from_schemata = false;
            $new_schema = null;
            if (
                is_string($schema)
                && ($new_schema = $schemata->schemaByName(
                    new AIngleName($schema, null, $defaultNamespace)
                ))
            ) {
                $is_schema_from_schemata = true;
            } else {
                $new_schema = self::subparse($schema, $defaultNamespace, $schemata);
            }

            $schemaType = $new_schema->type;
            if (
                self::isValidType($schemaType)
                && !self::isNamedType($schemaType)
                && in_array($schemaType, $schema_types)
            ) {
                throw new AIngleSchemaParseException(sprintf('"%s" is already in union', $schemaType));
            }

            if (AIngleSchema::UNION_SCHEMA === $schemaType) {
                throw new AIngleSchemaParseException('Unions cannot contain other unions');
            }

            $schema_types[] = $schemaType;
            $this->schemas[] = $new_schema;
            if ($is_schema_from_schemata) {
                $this->schemaFromSchemataIndices [] = $index;
            }
        }
    }

    /**
     * @returns AIngleSchema[]
     */
    public function schemas()
    {
        return $this->schemas;
    }

    /**
     * @returns AIngleSchema the particular schema from the union for
     * the given (zero-based) index.
     * @throws AIngleSchemaParseException if the index is invalid for this schema.
     */
    public function schemaByIndex($index)
    {
        if (count($this->schemas) > $index) {
            return $this->schemas[$index];
        }

        throw new AIngleSchemaParseException('Invalid union schema index');
    }

    /**
     * @returns mixed
     */
    public function toAIngle()
    {
        $aingle = array();

        foreach ($this->schemas as $index => $schema) {
            $aingle[] = in_array($index, $this->schemaFromSchemataIndices)
                ? $schema->qualifiedName()
                : $schema->toAIngle();
        }

        return $aingle;
    }
}
