

#!/usr/bin/env perl

use strict;
use warnings;
use Test::Exception;
use Test::More;

use_ok 'AIngle::Protocol';

{
    my $spec_proto = <<EOJ;
{
"namespace": "com.acme",
"protocol": "HelloWorld",
"doc": "Protocol Greetings",

"types": [
    {"name": "Greeting", "type": "record", "fields": [
        {"name": "message", "type": "string"}]},
    {"name": "Curse", "type": "error", "fields": [
        {"name": "message", "type": "string"}]}
],

"messages": {
    "hello": {
    "doc": "Say hello.",
    "request": [{"name": "greeting", "type": "Greeting" }],
    "response": "Greeting",
    "errors": ["Curse"]
    }
}
}
EOJ
    my $p = AIngle::Protocol->parse($spec_proto);
    ok $p, "proto returned";
    isa_ok $p, 'AIngle::Protocol';
    is $p->fullname, "com.acme.HelloWorld", "fullname";
    is $p->name, "HelloWorld", "name";
    is $p->namespace, "com.acme", "namespace";

    is $p->doc, "Protocol Greetings", "doc";

    isa_ok $p->types, 'HASH';
    isa_ok $p->types->{Greeting}, 'AIngle::Schema::Record';
    isa_ok $p->types->{Greeting}->fields_as_hash
           ->{message}{type}, 'AIngle::Schema::Primitive';

    isa_ok $p->messages->{hello}, "AIngle::Protocol::Message";
    is $p->messages->{hello}->doc, "Say hello.";
    isa_ok $p->messages->{hello}->errors, "AIngle::Schema::Union";
    isa_ok $p->messages->{hello}->response, "AIngle::Schema::Record";
    my $req_params = $p->messages->{hello}->request;
    isa_ok $req_params, "ARRAY";
    is scalar @$req_params, 1, "one parameter to hello message";
    is $req_params->[0]->{name}, "greeting", "greeting field";
    is $req_params->[0]->{type}, $p->types->{Greeting}, "same Schema type";
}

done_testing;
