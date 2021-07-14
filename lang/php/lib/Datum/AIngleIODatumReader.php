<?php

/**

 */

namespace Apache\AIngle\Datum;

use Apache\AIngle\AIngleException;
use Apache\AIngle\Schema\AIngleName;
use Apache\AIngle\Schema\AIngleSchema;

/**
 * Handles schema-specifc reading of data from the decoder.
 *
 * Also handles schema resolution between the reader and writer
 * schemas (if a writer's schema is provided).
 *
 * @package AIngle
 */
class AIngleIODatumReader
{
    /**
     * @var AIngleSchema
     */
    private $writers_schema;
    /**
     * @var AIngleSchema
     */
    private $readers_schema;

    /**
     * @param AIngleSchema $writers_schema
     * @param AIngleSchema $readers_schema
     */
    public function __construct($writers_schema = null, $readers_schema = null)
    {
        $this->writers_schema = $writers_schema;
        $this->readers_schema = $readers_schema;
    }

    /**
     * @param AIngleSchema $readers_schema
     */
    public function setWritersSchema($readers_schema)
    {
        $this->writers_schema = $readers_schema;
    }

    /**
     * @param AIngleIOBinaryDecoder $decoder
     * @returns string
     */
    public function read($decoder)
    {
        if (is_null($this->readers_schema)) {
            $this->readers_schema = $this->writers_schema;
        }
        return $this->readData(
            $this->writers_schema,
            $this->readers_schema,
            $decoder
        );
    }

    /**
     * @returns mixed
     */
    public function readData($writers_schema, $readers_schema, $decoder)
    {
        // Schema resolution: reader's schema is a union, writer's schema is not
        if (
            AIngleSchema::UNION_SCHEMA === $readers_schema->type()
            && AIngleSchema::UNION_SCHEMA !== $writers_schema->type()
        ) {
            foreach ($readers_schema->schemas() as $schema) {
                if (self::schemasMatch($writers_schema, $schema)) {
                    return $this->readData($writers_schema, $schema, $decoder);
                }
            }
            throw new AIngleIOSchemaMatchException($writers_schema, $readers_schema);
        }

        switch ($writers_schema->type()) {
            case AIngleSchema::NULL_TYPE:
                return $decoder->readNull();
            case AIngleSchema::BOOLEAN_TYPE:
                return $decoder->readBoolean();
            case AIngleSchema::INT_TYPE:
                return $decoder->readInt();
            case AIngleSchema::LONG_TYPE:
                return $decoder->readLong();
            case AIngleSchema::FLOAT_TYPE:
                return $decoder->readFloat();
            case AIngleSchema::DOUBLE_TYPE:
                return $decoder->readDouble();
            case AIngleSchema::STRING_TYPE:
                return $decoder->readString();
            case AIngleSchema::BYTES_TYPE:
                return $decoder->readBytes();
            case AIngleSchema::ARRAY_SCHEMA:
                return $this->readArray($writers_schema, $readers_schema, $decoder);
            case AIngleSchema::MAP_SCHEMA:
                return $this->readMap($writers_schema, $readers_schema, $decoder);
            case AIngleSchema::UNION_SCHEMA:
                return $this->readUnion($writers_schema, $readers_schema, $decoder);
            case AIngleSchema::ENUM_SCHEMA:
                return $this->readEnum($writers_schema, $readers_schema, $decoder);
            case AIngleSchema::FIXED_SCHEMA:
                return $this->readFixed($writers_schema, $readers_schema, $decoder);
            case AIngleSchema::RECORD_SCHEMA:
            case AIngleSchema::ERROR_SCHEMA:
            case AIngleSchema::REQUEST_SCHEMA:
                return $this->readRecord($writers_schema, $readers_schema, $decoder);
            default:
                throw new AIngleException(sprintf(
                    "Cannot read unknown schema type: %s",
                    $writers_schema->type()
                ));
        }
    }

    /**
     *
     * @param AIngleSchema $writers_schema
     * @param AIngleSchema $readers_schema
     * @returns boolean true if the schemas are consistent with
     *                  each other and false otherwise.
     */
    public static function schemasMatch($writers_schema, $readers_schema)
    {
        $writers_schema_type = $writers_schema->type;
        $readers_schema_type = $readers_schema->type;

        if (AIngleSchema::UNION_SCHEMA === $writers_schema_type || AIngleSchema::UNION_SCHEMA === $readers_schema_type) {
            return true;
        }

        if (AIngleSchema::isPrimitiveType($writers_schema_type)) {
            return true;
        }

        switch ($readers_schema_type) {
            case AIngleSchema::MAP_SCHEMA:
                return self::attributesMatch(
                    $writers_schema->values(),
                    $readers_schema->values(),
                    [AIngleSchema::TYPE_ATTR]
                );
            case AIngleSchema::ARRAY_SCHEMA:
                return self::attributesMatch(
                    $writers_schema->items(),
                    $readers_schema->items(),
                    [AIngleSchema::TYPE_ATTR]
                );
            case AIngleSchema::ENUM_SCHEMA:
                return self::attributesMatch(
                    $writers_schema,
                    $readers_schema,
                    [AIngleSchema::FULLNAME_ATTR]
                );
            case AIngleSchema::FIXED_SCHEMA:
                return self::attributesMatch(
                    $writers_schema,
                    $readers_schema,
                    [
                        AIngleSchema::FULLNAME_ATTR,
                        AIngleSchema::SIZE_ATTR
                    ]
                );
            case AIngleSchema::RECORD_SCHEMA:
            case AIngleSchema::ERROR_SCHEMA:
                return self::attributesMatch(
                    $writers_schema,
                    $readers_schema,
                    [AIngleSchema::FULLNAME_ATTR]
                );
            case AIngleSchema::REQUEST_SCHEMA:
                // XXX: This seems wrong
                return true;
            // XXX: no default
        }

        if (
            AIngleSchema::INT_TYPE === $writers_schema_type
            && in_array($readers_schema_type, [
                AIngleSchema::LONG_TYPE,
                AIngleSchema::FLOAT_TYPE,
                AIngleSchema::DOUBLE_TYPE
            ])
        ) {
            return true;
        }

        if (
            AIngleSchema::LONG_TYPE === $writers_schema_type
            && in_array($readers_schema_type, [
                AIngleSchema::FLOAT_TYPE,
                AIngleSchema::DOUBLE_TYPE
            ])
        ) {
            return true;
        }

        if (AIngleSchema::FLOAT_TYPE === $writers_schema_type && AIngleSchema::DOUBLE_TYPE === $readers_schema_type) {
            return true;
        }

        return false;
    }

    /**
     * Checks equivalence of the given attributes of the two given schemas.
     *
     * @param AIngleSchema $schema_one
     * @param AIngleSchema $schema_two
     * @param string[] $attribute_names array of string attribute names to compare
     *
     * @return boolean true if the attributes match and false otherwise.
     */
    public static function attributesMatch($schema_one, $schema_two, $attribute_names)
    {
        foreach ($attribute_names as $attribute_name) {
            if ($schema_one->attribute($attribute_name) !== $schema_two->attribute($attribute_name)) {
                if ($attribute_name === AIngleSchema::FULLNAME_ATTR) {
                    foreach ($schema_two->getAliases() as $alias) {
                        if (
                            $schema_one->attribute($attribute_name) === (new AIngleName(
                                $alias,
                                $schema_two->attribute(AIngleSchema::NAMESPACE_ATTR),
                                null
                            ))->fullname()
                        ) {
                            return true;
                        }
                    }
                }
                return false;
            }
        }
        return true;
    }

    /**
     * @return array
     */
    public function readArray($writers_schema, $readers_schema, $decoder)
    {
        $items = array();
        $block_count = $decoder->readLong();
        while (0 !== $block_count) {
            if ($block_count < 0) {
                $block_count = -$block_count;
                $block_size = $decoder->readLong(); // Read (and ignore) block size
            }
            for ($i = 0; $i < $block_count; $i++) {
                $items [] = $this->readData(
                    $writers_schema->items(),
                    $readers_schema->items(),
                    $decoder
                );
            }
            $block_count = $decoder->readLong();
        }
        return $items;
    }

    /**
     * @returns array
     */
    public function readMap($writers_schema, $readers_schema, $decoder)
    {
        $items = array();
        $pair_count = $decoder->readLong();
        while (0 != $pair_count) {
            if ($pair_count < 0) {
                $pair_count = -$pair_count;
                // Note: we're not doing anything with block_size other than skipping it
                $block_size = $decoder->readLong();
            }

            for ($i = 0; $i < $pair_count; $i++) {
                $key = $decoder->readString();
                $items[$key] = $this->readData(
                    $writers_schema->values(),
                    $readers_schema->values(),
                    $decoder
                );
            }
            $pair_count = $decoder->readLong();
        }
        return $items;
    }

    /**
     * @returns mixed
     */
    public function readUnion($writers_schema, $readers_schema, $decoder)
    {
        $schema_index = $decoder->readLong();
        $selected_writers_schema = $writers_schema->schemaByIndex($schema_index);
        return $this->readData($selected_writers_schema, $readers_schema, $decoder);
    }

    /**
     * @returns string
     */
    public function readEnum($writers_schema, $readers_schema, $decoder)
    {
        $symbol_index = $decoder->readInt();
        $symbol = $writers_schema->symbolByIndex($symbol_index);
        if (!$readers_schema->hasSymbol($symbol)) {
            null;
        } // FIXME: unset wrt schema resolution
        return $symbol;
    }

    /**
     * @returns string
     */
    public function readFixed($writers_schema, $readers_schema, $decoder)
    {
        return $decoder->read($writers_schema->size());
    }

    /**
     * @returns array
     */
    public function readRecord($writers_schema, $readers_schema, $decoder)
    {
        $readers_fields = $readers_schema->fieldsHash();
        $record = [];
        foreach ($writers_schema->fields() as $writers_field) {
            $type = $writers_field->type();
            $readers_field = $readers_fields[$writers_field->name()] ?? null;
            if ($readers_field) {
                $record[$writers_field->name()] = $this->readData($type, $readers_field->type(), $decoder);
            } elseif (isset($readers_schema->fieldsByAlias()[$writers_field->name()])) {
                $readers_field = $readers_schema->fieldsByAlias()[$writers_field->name()];
                $field_val = $this->readData($writers_field->type(), $readers_field->type(), $decoder);
                $record[$readers_field->name()] = $field_val;
            } else {
                self::skipData($type, $decoder);
            }
        }
        // Fill in default values
        foreach ($readers_fields as $field_name => $field) {
            if (isset($writers_fields[$field_name])) {
                continue;
            }
            if ($field->hasDefaultValue()) {
                $record[$field->name()] = $this->readDefaultValue($field->type(), $field->defaultValue());
            } else {
                null;
            }
        }

        return $record;
    }

    /**
     * @param AIngleSchema $writers_schema
     * @param AIngleIOBinaryDecoder $decoder
     */
    public static function skipData($writers_schema, $decoder)
    {
        switch ($writers_schema->type()) {
            case AIngleSchema::NULL_TYPE:
                return $decoder->skipNull();
            case AIngleSchema::BOOLEAN_TYPE:
                return $decoder->skipBoolean();
            case AIngleSchema::INT_TYPE:
                return $decoder->skipInt();
            case AIngleSchema::LONG_TYPE:
                return $decoder->skipLong();
            case AIngleSchema::FLOAT_TYPE:
                return $decoder->skipFloat();
            case AIngleSchema::DOUBLE_TYPE:
                return $decoder->skipDouble();
            case AIngleSchema::STRING_TYPE:
                return $decoder->skipString();
            case AIngleSchema::BYTES_TYPE:
                return $decoder->skipBytes();
            case AIngleSchema::ARRAY_SCHEMA:
                return $decoder->skipArray($writers_schema, $decoder);
            case AIngleSchema::MAP_SCHEMA:
                return $decoder->skipMap($writers_schema, $decoder);
            case AIngleSchema::UNION_SCHEMA:
                return $decoder->skipUnion($writers_schema, $decoder);
            case AIngleSchema::ENUM_SCHEMA:
                return $decoder->skipEnum($writers_schema, $decoder);
            case AIngleSchema::FIXED_SCHEMA:
                return $decoder->skipFixed($writers_schema, $decoder);
            case AIngleSchema::RECORD_SCHEMA:
            case AIngleSchema::ERROR_SCHEMA:
            case AIngleSchema::REQUEST_SCHEMA:
                return $decoder->skipRecord($writers_schema, $decoder);
            default:
                throw new AIngleException(sprintf(
                    'Unknown schema type: %s',
                    $writers_schema->type()
                ));
        }
    }

    /**
     * @param AIngleSchema $field_schema
     * @param null|boolean|int|float|string|array $default_value
     * @returns null|boolean|int|float|string|array
     *
     * @throws AIngleException if $field_schema type is unknown.
     */
    public function readDefaultValue($field_schema, $default_value)
    {
        switch ($field_schema->type()) {
            case AIngleSchema::NULL_TYPE:
                return null;
            case AIngleSchema::BOOLEAN_TYPE:
                return $default_value;
            case AIngleSchema::INT_TYPE:
            case AIngleSchema::LONG_TYPE:
                return (int) $default_value;
            case AIngleSchema::FLOAT_TYPE:
            case AIngleSchema::DOUBLE_TYPE:
                return (float) $default_value;
            case AIngleSchema::STRING_TYPE:
            case AIngleSchema::BYTES_TYPE:
                return $default_value;
            case AIngleSchema::ARRAY_SCHEMA:
                $array = array();
                foreach ($default_value as $json_val) {
                    $val = $this->readDefaultValue($field_schema->items(), $json_val);
                    $array [] = $val;
                }
                return $array;
            case AIngleSchema::MAP_SCHEMA:
                $map = array();
                foreach ($default_value as $key => $json_val) {
                    $map[$key] = $this->readDefaultValue(
                        $field_schema->values(),
                        $json_val
                    );
                }
                return $map;
            case AIngleSchema::UNION_SCHEMA:
                return $this->readDefaultValue(
                    $field_schema->schemaByIndex(0),
                    $default_value
                );
            case AIngleSchema::ENUM_SCHEMA:
            case AIngleSchema::FIXED_SCHEMA:
                return $default_value;
            case AIngleSchema::RECORD_SCHEMA:
                $record = array();
                foreach ($field_schema->fields() as $field) {
                    $field_name = $field->name();
                    if (!$json_val = $default_value[$field_name]) {
                        $json_val = $field->default_value();
                    }

                    $record[$field_name] = $this->readDefaultValue(
                        $field->type(),
                        $json_val
                    );
                }
                return $record;
            default:
                throw new AIngleException(sprintf('Unknown type: %s', $field_schema->type()));
        }
    }
}
