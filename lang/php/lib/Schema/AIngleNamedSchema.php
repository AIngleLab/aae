<?php

/**

 */

namespace Apache\AIngle\Schema;

/**
 * Parent class of named AIngle schema
 * @package AIngle
 * @todo Refactor AIngleNamedSchema to use an AIngleName instance
 *       to store name information.
 */
class AIngleNamedSchema extends AIngleSchema
{
    /**
     * @var AIngleName $name
     */
    private $name;

    /**
     * @var string documentation string
     */
    private $doc;
    /**
     * @var array
     */
    private $aliases;

    /**
     * @param string $type
     * @param AIngleName $name
     * @param string $doc documentation string
     * @param AIngleNamedSchemata &$schemata
     * @param array $aliases
     * @throws AIngleSchemaParseException
     */
    public function __construct($type, $name, $doc = null, &$schemata = null, $aliases = null)
    {
        parent::__construct($type);
        $this->name = $name;

        if ($doc && !is_string($doc)) {
            throw new AIngleSchemaParseException('Schema doc attribute must be a string');
        }
        $this->doc = $doc;
        if ($aliases) {
            self::hasValidAliases($aliases);
            $this->aliases = $aliases;
        }

        if (!is_null($schemata)) {
            $schemata = $schemata->cloneWithNewSchema($this);
        }
    }

    public function getAliases()
    {
        return $this->aliases;
    }

    /**
     * @returns mixed
     */
    public function toAIngle()
    {
        $aingle = parent::toAIngle();
        [$name, $namespace] = AIngleName::extractNamespace($this->qualifiedName());
        $aingle[AIngleSchema::NAME_ATTR] = $name;
        if ($namespace) {
            $aingle[AIngleSchema::NAMESPACE_ATTR] = $namespace;
        }
        if (!is_null($this->doc)) {
            $aingle[AIngleSchema::DOC_ATTR] = $this->doc;
        }
        if (!is_null($this->aliases)) {
            $aingle[AIngleSchema::ALIASES_ATTR] = $this->aliases;
        }
        return $aingle;
    }

    public function qualifiedName()
    {
        return $this->name->qualifiedName();
    }

    /**
     * @returns string
     */
    public function fullname()
    {
        return $this->name->fullname();
    }
}
