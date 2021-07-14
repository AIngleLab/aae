<?php

/**

 */

namespace Apache\AIngle\Datum;

use Apache\AIngle\AIngleException;
use Apache\AIngle\Schema\AIngleSchema;

/**
 * Handles schema-specific writing of data to the encoder.
 *
 * Ensures that each datum written is consistent with the writer's schema.
 *
 * @package AIngle
 */
class AIngleIODatumWriter
{
    /**
     * Schema used by this instance to write AIngle data.
     * @var AIngleSchema
     */
    public $writersSchema;

    /**
     * @param AIngleSchema $writers_schema
     */
    public function __construct($writers_schema = null)
    {
        $this->writersSchema = $writers_schema;
    }

    /**
     * @param $datum
     * @param AIngleIOBinaryEncoder $encoder
     */
    public function write($datum, $encoder)
    {
        $this->writeData($this->writersSchema, $datum, $encoder);
    }

    /**
     * @param AIngleSchema $writers_schema
     * @param $datum
     * @param AIngleIOBinaryEncoder $encoder
     * @returns mixed
     *
     * @throws AIngleIOTypeException if $datum is invalid for $writers_schema
     */
    public function writeData($writers_schema, $datum, $encoder)
    {
        if (!AIngleSchema::isValidDatum($writers_schema, $datum)) {
            throw new AIngleIOTypeException($writers_schema, $datum);
        }

        switch ($writers_schema->type()) {
            case AIngleSchema::NULL_TYPE:
                return $encoder->writeNull($datum);
            case AIngleSchema::BOOLEAN_TYPE:
                return $encoder->writeBoolean($datum);
            case AIngleSchema::INT_TYPE:
                return $encoder->writeInt($datum);
            case AIngleSchema::LONG_TYPE:
                return $encoder->writeLong($datum);
            case AIngleSchema::FLOAT_TYPE:
                return $encoder->writeFloat($datum);
            case AIngleSchema::DOUBLE_TYPE:
                return $encoder->writeDouble($datum);
            case AIngleSchema::STRING_TYPE:
                return $encoder->writeString($datum);
            case AIngleSchema::BYTES_TYPE:
                return $encoder->writeBytes($datum);
            case AIngleSchema::ARRAY_SCHEMA:
                return $this->writeArray($writers_schema, $datum, $encoder);
            case AIngleSchema::MAP_SCHEMA:
                return $this->writeMap($writers_schema, $datum, $encoder);
            case AIngleSchema::FIXED_SCHEMA:
                return $this->writeFixed($writers_schema, $datum, $encoder);
            case AIngleSchema::ENUM_SCHEMA:
                return $this->writeEnum($writers_schema, $datum, $encoder);
            case AIngleSchema::RECORD_SCHEMA:
            case AIngleSchema::ERROR_SCHEMA:
            case AIngleSchema::REQUEST_SCHEMA:
                return $this->writeRecord($writers_schema, $datum, $encoder);
            case AIngleSchema::UNION_SCHEMA:
                return $this->writeUnion($writers_schema, $datum, $encoder);
            default:
                throw new AIngleException(sprintf(
                    'Unknown type: %s',
                    $writers_schema->type
                ));
        }
    }

    /**
     * @param AIngleSchema $writers_schema
     * @param null|boolean|int|float|string|array $datum item to be written
     * @param AIngleIOBinaryEncoder $encoder
     */
    private function writeArray($writers_schema, $datum, $encoder)
    {
        $datum_count = count($datum);
        if (0 < $datum_count) {
            $encoder->writeLong($datum_count);
            $items = $writers_schema->items();
            foreach ($datum as $item) {
                $this->writeData($items, $item, $encoder);
            }
        }
        return $encoder->writeLong(0);
    }

    /**
     * @param $writers_schema
     * @param $datum
     * @param $encoder
     * @throws AIngleIOTypeException
     */
    private function writeMap($writers_schema, $datum, $encoder)
    {
        $datum_count = count($datum);
        if ($datum_count > 0) {
            $encoder->writeLong($datum_count);
            foreach ($datum as $k => $v) {
                $encoder->writeString($k);
                $this->writeData($writers_schema->values(), $v, $encoder);
            }
        }
        $encoder->writeLong(0);
    }

    private function writeFixed($writers_schema, $datum, $encoder)
    {
        /**
         * NOTE Unused $writers_schema parameter included for consistency
         * with other write_* methods.
         */
        return $encoder->write($datum);
    }

    private function writeEnum($writers_schema, $datum, $encoder)
    {
        $datum_index = $writers_schema->symbolIndex($datum);
        return $encoder->writeInt($datum_index);
    }

    private function writeRecord($writers_schema, $datum, $encoder)
    {
        foreach ($writers_schema->fields() as $field) {
            $this->writeData($field->type(), $datum[$field->name()] ?? null, $encoder);
        }
    }

    private function writeUnion($writers_schema, $datum, $encoder)
    {
        $datum_schema_index = -1;
        $datum_schema = null;
        foreach ($writers_schema->schemas() as $index => $schema) {
            if (AIngleSchema::isValidDatum($schema, $datum)) {
                $datum_schema_index = $index;
                $datum_schema = $schema;
                break;
            }
        }

        if (is_null($datum_schema)) {
            throw new AIngleIOTypeException($writers_schema, $datum);
        }

        $encoder->writeLong($datum_schema_index);
        $this->writeData($datum_schema, $datum, $encoder);
    }
}
