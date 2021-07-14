<?php

/**

 */

namespace Apache\AIngle\Protocol;

use Apache\AIngle\Schema\AIngleNamedSchemata;
use Apache\AIngle\Schema\AIngleSchema;

/**
 * AIngle library for protocols
 * @package AIngle
 */
class AIngleProtocol
{
    public $name;
    public $namespace;
    public $schemata;
    public $messages;

    public static function parse($json)
    {
        if (is_null($json)) {
            throw new AIngleProtocolParseException("Protocol can't be null");
        }

        $protocol = new AIngleProtocol();
        $protocol->realParse(json_decode($json, true));
        return $protocol;
    }

    public function realParse($aingle)
    {
        $this->protocol = $aingle["protocol"];
        $this->namespace = $aingle["namespace"];
        $this->schemata = new AIngleNamedSchemata();
        $this->name = $aingle["protocol"];

        if (!is_null($aingle["types"])) {
            $types = AIngleSchema::realParse($aingle["types"], $this->namespace, $this->schemata);
        }

        if (!is_null($aingle["messages"])) {
            foreach ($aingle["messages"] as $messageName => $messageAIngle) {
                $message = new AIngleProtocolMessage($messageName, $messageAIngle, $this);
                $this->messages[$messageName] = $message;
            }
        }
    }
}
