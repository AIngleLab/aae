<?php

/**

 */

namespace Apache\AIngle\Schema;

/**
 *  Keeps track of AIngleNamedSchema which have been observed so far,
 *  as well as the default namespace.
 *
 * @package AIngle
 */
class AIngleNamedSchemata
{
    /**
     * @var AIngleNamedSchema[]
     */
    private $schemata;

    /**
     * @param AIngleNamedSchemata[]
     */
    public function __construct($schemata = array())
    {
        $this->schemata = $schemata;
    }

    public function listSchemas()
    {
        var_export($this->schemata);
        foreach ($this->schemata as $sch) {
            print('Schema ' . $sch->__toString() . "\n");
        }
    }

    /**
     * @param AIngleName $name
     * @returns AIngleSchema|null
     */
    public function schemaByName($name)
    {
        return $this->schema($name->fullname());
    }

    /**
     * @param string $fullname
     * @returns AIngleSchema|null the schema which has the given name,
     *          or null if there is no schema with the given name.
     */
    public function schema($fullname)
    {
        if (isset($this->schemata[$fullname])) {
            return $this->schemata[$fullname];
        }
        return null;
    }

    /**
     * Creates a new AIngleNamedSchemata instance of this schemata instance
     * with the given $schema appended.
     * @param AIngleNamedSchema schema to add to this existing schemata
     * @returns AIngleNamedSchemata
     */
    public function cloneWithNewSchema($schema)
    {
        $name = $schema->fullname();
        if (AIngleSchema::isValidType($name)) {
            throw new AIngleSchemaParseException(sprintf('Name "%s" is a reserved type name', $name));
        }
        if ($this->hasName($name)) {
            throw new AIngleSchemaParseException(sprintf('Name "%s" is already in use', $name));
        }
        $schemata = new AIngleNamedSchemata($this->schemata);
        $schemata->schemata[$name] = $schema;
        return $schemata;
    }

    /**
     * @param string $fullname
     * @returns boolean true if there exists a schema with the given name
     *                  and false otherwise.
     */
    public function hasName($fullname)
    {
        return array_key_exists($fullname, $this->schemata);
    }
}
