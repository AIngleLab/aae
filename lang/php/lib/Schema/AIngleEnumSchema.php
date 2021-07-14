<?php

/**

 */

namespace Apache\AIngle\Schema;

use Apache\AIngle\AIngleException;
use Apache\AIngle\AIngleUtil;

/**
 * @package AIngle
 */
class AIngleEnumSchema extends AIngleNamedSchema
{
    /**
     * @var string[] array of symbols
     */
    private $symbols;

    /**
     * @param AIngleName $name
     * @param string $doc
     * @param string[] $symbols
     * @param AIngleNamedSchemata &$schemata
     * @throws AIngleSchemaParseException
     */
    public function __construct($name, $doc, $symbols, &$schemata = null, $aliases = null)
    {
        if (!AIngleUtil::isList($symbols)) {
            throw new AIngleSchemaParseException('Enum Schema symbols are not a list');
        }

        if (count(array_unique($symbols)) > count($symbols)) {
            throw new AIngleSchemaParseException(
                sprintf('Duplicate symbols: %s', $symbols)
            );
        }

        foreach ($symbols as $symbol) {
            if (!is_string($symbol) || empty($symbol)) {
                throw new AIngleSchemaParseException(
                    sprintf('Enum schema symbol must be a string %s', print_r($symbol, true))
                );
            }
        }

        parent::__construct(AIngleSchema::ENUM_SCHEMA, $name, $doc, $schemata, $aliases);
        $this->symbols = $symbols;
    }

    /**
     * @returns string[] this enum schema's symbols
     */
    public function symbols()
    {
        return $this->symbols;
    }

    /**
     * @param string $symbol
     * @returns boolean true if the given symbol exists in this
     *          enum schema and false otherwise
     */
    public function hasSymbol($symbol)
    {
        return in_array($symbol, $this->symbols);
    }

    /**
     * @param int $index
     * @returns string enum schema symbol with the given (zero-based) index
     */
    public function symbolByIndex($index)
    {
        if (array_key_exists($index, $this->symbols)) {
            return $this->symbols[$index];
        }
        throw new AIngleException(sprintf('Invalid symbol index %d', $index));
    }

    /**
     * @param string $symbol
     * @returns int the index of the given $symbol in the enum schema
     */
    public function symbolIndex($symbol)
    {
        $idx = array_search($symbol, $this->symbols, true);
        if (false !== $idx) {
            return $idx;
        }
        throw new AIngleException(sprintf("Invalid symbol value '%s'", $symbol));
    }

    /**
     * @returns mixed
     */
    public function toAIngle()
    {
        $aingle = parent::toAIngle();
        $aingle[AIngleSchema::SYMBOLS_ATTR] = $this->symbols;
        return $aingle;
    }
}
