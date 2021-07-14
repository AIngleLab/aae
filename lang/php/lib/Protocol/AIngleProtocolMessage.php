<?php

/**

 */

namespace Apache\AIngle\Protocol;

use Apache\AIngle\Schema\AIngleName;
use Apache\AIngle\Schema\AInglePrimitiveSchema;
use Apache\AIngle\Schema\AIngleRecordSchema;
use Apache\AIngle\Schema\AIngleSchema;

class AIngleProtocolMessage
{
    /**
     * @var AIngleRecordSchema $request
     */
    public $request;

    public $response;

    public function __construct($name, $aingle, $protocol)
    {
        $this->name = $name;
        $this->request = new AIngleRecordSchema(
            new AIngleName($name, null, $protocol->namespace),
            null,
            $aingle['request'],
            $protocol->schemata,
            AIngleSchema::REQUEST_SCHEMA
        );

        if (array_key_exists('response', $aingle)) {
            $this->response = $protocol->schemata->schemaByName(new AIngleName(
                $aingle['response'],
                $protocol->namespace,
                $protocol->namespace
            ));
            if ($this->response == null) {
                $this->response = new AInglePrimitiveSchema($aingle['response']);
            }
        }
    }
}
