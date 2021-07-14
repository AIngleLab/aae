<?php
/**

 */

namespace Apache\AIngle\Tests;

use Apache\AIngle\Schema\AIngleName;
use Apache\AIngle\Schema\AIngleSchemaParseException;
use PHPUnit\Framework\TestCase;

class NameExample
{
    var $is_valid;
    var $name;
    var $namespace;
    var $default_namespace;
    var $expected_fullname;

    function __construct(
        $name,
        $namespace,
        $default_namespace,
        $is_valid,
        $expected_fullname = null
    ) {
        $this->name = $name;
        $this->namespace = $namespace;
        $this->default_namespace = $default_namespace;
        $this->is_valid = $is_valid;
        $this->expected_fullname = $expected_fullname;
    }

    function __toString()
    {
        return var_export($this, true);
    }
}

class NameTest extends TestCase
{

    function fullname_provider()
    {
        $examples = array(
            new NameExample('foo', null, null, true, 'foo'),
            new NameExample('foo', 'bar', null, true, 'bar.foo'),
            new NameExample('bar.foo', 'baz', null, true, 'bar.foo'),
            new NameExample('_bar.foo', 'baz', null, true, '_bar.foo'),
            new NameExample('bar._foo', 'baz', null, true, 'bar._foo'),
            new NameExample('3bar.foo', 'baz', null, false),
            new NameExample('bar.3foo', 'baz', null, false),
            new NameExample('b4r.foo', 'baz', null, true, 'b4r.foo'),
            new NameExample('bar.f0o', 'baz', null, true, 'bar.f0o'),
            new NameExample(' .foo', 'baz', null, false),
            new NameExample('bar. foo', 'baz', null, false),
            new NameExample('bar. ', 'baz', null, false)
        );
        $exes = array();
        foreach ($examples as $ex) {
            $exes [] = array($ex);
        }
        return $exes;
    }

    /**
     * @dataProvider fullname_provider
     */
    function test_fullname($ex)
    {
        try {
            $name = new AIngleName($ex->name, $ex->namespace, $ex->default_namespace);
            $this->assertTrue($ex->is_valid);
            $this->assertEquals($ex->expected_fullname, $name->fullname());
        } catch (AIngleSchemaParseException $e) {
            $this->assertFalse($ex->is_valid, sprintf("%s:\n%s",
                $ex,
                $e->getMessage()));
        }
    }

    function name_provider()
    {
        return [
            ['a', true],
            ['_', true],
            ['1a', false],
            ['', false],
            [null, false],
            [' ', false],
            ['Cons', true]
        ];
    }

    /**
     * @dataProvider name_provider
     */
    function test_name($name, $is_well_formed)
    {
        $this->assertEquals(AIngleName::isWellFormedName($name), $is_well_formed);
    }
}
