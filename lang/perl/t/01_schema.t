

use strict;
use warnings;

use Test::More;
plan tests => 130;
use Test::Exception;
use_ok 'AIngle::Schema';

dies_ok { AIngle::Schema->new } "Should use parse() or instantiate the subclass";

throws_ok { AIngle::Schema->parse(q()) } "AIngle::Schema::Error::Parse";
throws_ok { AIngle::Schema->parse(q(test)) } "AIngle::Schema::Error::Parse";
throws_ok { AIngle::Schema->parse(q({"type": t})) }
            "AIngle::Schema::Error::Parse";
throws_ok { AIngle::Schema->parse(q({"type": t})) }
            "AIngle::Schema::Error::Parse";

my $s = AIngle::Schema->parse(q("string"));
isa_ok $s, 'AIngle::Schema::Base';
isa_ok $s, 'AIngle::Schema::Primitive',
is $s->type, "string", "type is string";

my $s2 = AIngle::Schema->parse(q({"type": "string"}));
isa_ok $s2, 'AIngle::Schema::Primitive';
is $s2->type, "string", "type is string";
is $s, $s2, "string Schematas are singletons";

## Records
{
    my $s3 = AIngle::Schema::Record->new(
        struct => {
            name => 'saucisson',
            fields => [
                { name => 'a', type => 'long'   },
                { name => 'b', type => 'string' },
            ],
        },
    );

    isa_ok $s3, 'AIngle::Schema::Record';
    is $s3->type, 'record', "this is a record type";
    is $s3->fullname, 'saucisson', "correct name";
    is $s3->fields->[0]{name}, 'a', 'a';
    is $s3->fields->[0]{type}, AIngle::Schema::Primitive->new(type => 'long'), 'long';
    is $s3->fields->[1]{name}, 'b', 'b';
    is $s3->fields->[1]{type}, AIngle::Schema::Primitive->new(type => 'string'), 'str';

    ## self-reference
    $s3 = AIngle::Schema::Record->new(
        struct => {
            name => 'saucisson',
            fields => [
                { name => 'a', type => 'long'      },
                { name => 'b', type => 'saucisson' },
            ],
        },
    );
    isa_ok $s3, 'AIngle::Schema::Record';
    is $s3->fullname, 'saucisson', "correct name";
    is $s3->fields->[0]{name}, 'a', 'a';
    is $s3->fields->[0]{type}, AIngle::Schema::Primitive->new(type => 'long'), 'long';
    is $s3->fields->[1]{name}, 'b', 'b';
    is $s3->fields->[1]{type}, $s3, 'self!';

    ## serialize
    my $string = $s3->to_string;
    like $string, qr/saucisson/, "generated string has 'saucisson'";
    my $s3bis = AIngle::Schema->parse($string);
    is_deeply $s3bis->to_struct, $s3->to_struct,
        'regenerated structure matches original';

    ## record fields can have defaults
    my @good_ints = (2, -1, -(2**31 - 1), 2_147_483_647, "2147483647"  );
    my @bad_ints = ("", "string", 9.22337204, 9.22337204E10, \"2");
    my @good_longs = (1, 2, -3);
    my @bad_longs = (9.22337204, 9.22337204E10 + 0.1, \"2");

    use Config;
    if ($Config{use64bitint}) {
        push @bad_ints, (2**32 - 1, 4_294_967_296, 9_223_372_036_854_775_807);
        push @good_longs, (9_223_372_036_854_775_807, 3e10);
        push @bad_longs, 9_223_372_036_854_775_808;
    }
    else {
        require Math::BigInt;
        push @bad_ints, map { Math::BigInt->new($_) }
            ("0xFFFF_FFFF", "0x1_0000_0000", "0x7FFF_FFFF_FFFF_FFFF");
        push @good_longs, map { Math::BigInt->new($_) }
            ("9_223_372_036_854_775_807", "3e10");
        push @bad_longs, Math::BigInt->new("9_223_372_036_854_775_808");
    }

    for (@good_ints) {
        my $s4 = AIngle::Schema::Record->new(
            struct => { name => 'saucisson',
                fields => [
                    { name => 'a', type => 'int', default => $_ },
                ],
            },
        );
        is $s4->fields->[0]{default}, $_, "default $_";
    }
    for (@good_longs) {
        my $s4 = AIngle::Schema::Record->new(
            struct => { name => 'saucisson',
                fields => [
                    { name => 'a', type => 'long', default => $_ },
                ],
            },
        );
        is $s4->fields->[0]{default}, $_, "default $_";
    }
    for (@bad_ints) {
        throws_ok  { AIngle::Schema::Record->new(
            struct => { name => 'saucisson',
                fields => [
                    { name => 'a', type => 'int', default => $_ },
                ],
            },
        ) } "AIngle::Schema::Error::Parse", "invalid default: $_";
    }
    for (@bad_longs) {
        throws_ok  { AIngle::Schema::Record->new(
            struct => { name => 'saucisson',
                fields => [
                    { name => 'a', type => 'long', default => $_ },
                ],
            },
        ) } "AIngle::Schema::Error::Parse", "invalid default: $_";
    }

    ## default of more complex types
    throws_ok {
        AIngle::Schema::Record->new(
            struct => { name => 'saucisson',
                fields => [
                    { name => 'a', type => 'union', default => 1 },
                ],
            },
        )
    } "AIngle::Schema::Error::Parse", "union don't have default: $@";

    my $s4 = AIngle::Schema->parse_struct(
        {
            type => 'record',
            name => 'saucisson',
            fields => [
                { name => 'string', type => 'string', default => "something" },
                { name => 'map', type => { type => 'map', values => 'long' }, default => {a => 2} },
                { name => 'array', type => { type => 'array', items => 'long' }, default => [1, 2] },
                { name => 'bytes', type => 'bytes', default => "something" },
                { name => 'null', type => 'null', default => undef },
            ],
        },
    );
    is $s4->fields->[0]{default}, "something", "string default";
    is_deeply $s4->fields->[1]{default}, { a => 2 }, "map default";
    is_deeply $s4->fields->[2]{default}, [1, 2], "array default";
    is $s4->fields->[3]{default}, "something", "bytes default";
    is $s4->fields->[4]{default}, undef, "null default";
    ## TODO: technically we should verify that default map/array match values
    ## and items types defined

    ## ordering
    for (qw(ascending descending ignore)) {
        my $s4 = AIngle::Schema::Record->new(
            struct => {
                name => 'saucisson',
                fields => [
                    { name => 'a', type => 'int', order => $_ },
                ],
            },
        );
        is $s4->fields->[0]{order}, $_, "order set to $_";
    }
    for (qw(DESCEND ascend DESCENDING ASCENDING)) {
        throws_ok  { AIngle::Schema::Record->new(
            struct => { name => 'saucisson',
                fields => [
                    { name => 'a', type => 'long', order => $_ },
                ],
            },
        ) } "AIngle::Schema::Error::Parse", "invalid order: $_";
    }
}

## Unions
{
    my $spec_example = <<EOJ;
{
  "type": "record",
  "name": "LongList",
  "fields" : [
    {"name": "value", "type": "long"},
    {"name": "next", "type": ["LongList", "null"]}
  ]
}
EOJ
    my $schema = AIngle::Schema->parse($spec_example);
    is $schema->type, 'record', "type record";
    is $schema->fullname, 'LongList', "name is LongList";

    ## Union checks
    # can only contain one type

    $s = <<EOJ;
["null", "null"]
EOJ
    throws_ok { AIngle::Schema->parse($s) }
              'AIngle::Schema::Error::Parse';

    $s = <<EOJ;
["long", "string", "float", "string"]
EOJ
    throws_ok { AIngle::Schema->parse($s) }
              'AIngle::Schema::Error::Parse';

    $s = <<EOJ;
{
  "type": "record",
  "name": "embed",
  "fields": [
    {"name": "value", "type":
        { "type": "record", "name": "rec1",  "fields": [
            { "name": "str1", "type": "string"}
        ] }
    },
    {"name": "next", "type": ["embed", "rec1", "embed"] }
  ]
}
EOJ
    throws_ok { AIngle::Schema->parse($s) }
          'AIngle::Schema::Error::Parse',
          'two records with same name in the union';

    $s = <<EOJ;
{
  "type": "record",
  "name": "embed",
  "fields": [
    {"name": "value", "type":
        { "type": "record", "name": "rec1",  "fields": [
            { "name": "str1", "type": "string"}
        ] }
    },
    {"name": "next", "type": ["embed", "rec1"] }
  ]
}
EOJ
    lives_ok { AIngle::Schema->parse($s) }
             'two records of different names in the union';

    # cannot directly embed another union
    $s = <<EOJ;
["long", ["string", "float"], "string"]
EOJ
    throws_ok { AIngle::Schema->parse($s) }
             'AIngle::Schema::Error::Parse', "cannot embed union in union";
}

## Enums!
{
    my $s = <<EOJ;
{ "type": "enum", "name": "theenum", "symbols": [ "A", "B" ]}
EOJ
    my $schema = AIngle::Schema->parse($s);
    is $schema->type, 'enum', "enum";
    is $schema->fullname, 'theenum', "fullname";
    is $schema->symbols->[0], "A", "symbol A";
    is $schema->symbols->[1], "B", "symbol B";
    my $string = $schema->to_string;
    my $s2 = AIngle::Schema->parse($string)->to_struct;
    is_deeply $s2, $schema->to_struct, "reserialized identically";
}

## Arrays
{
    my $s = <<EOJ;
{ "type": "array", "items": "string" }
EOJ
    my $schema = AIngle::Schema->parse($s);
    is $schema->type, 'array', "array";
    isa_ok $schema->items, 'AIngle::Schema::Primitive';
    is $schema->items->type, 'string', "type of items is string";
    my $string = $schema->to_string;
    my $s2 = AIngle::Schema->parse($string);
    is_deeply $s2, $schema, "reserialized identically";
}

## Maps
{
    my $s = <<EOJ;
{ "type": "map", "values": "string" }
EOJ
    my $schema = AIngle::Schema->parse($s);
    is $schema->type, 'map', "map";
    isa_ok $schema->values, 'AIngle::Schema::Primitive';
    is $schema->values->type, 'string', "type of values is string";
    my $string = $schema->to_string;
    my $s2 = AIngle::Schema->parse($string);
    is_deeply $s2, $schema, "reserialized identically";
}

## Fixed
{
    my $s = <<EOJ;
{ "type": "fixed", "name": "somefixed", "size": "something" }
EOJ
    throws_ok { AIngle::Schema->parse($s) } "AIngle::Schema::Error::Parse",
        "size must be an int";

    $s = <<EOJ;
{ "type": "fixed", "name": "somefixed", "size": -100 }
EOJ
    throws_ok { AIngle::Schema->parse($s) } "AIngle::Schema::Error::Parse",
        "size must be a POSITIVE int";

    $s = <<EOJ;
{ "type": "fixed", "name": "somefixed", "size": 0 }
EOJ
    throws_ok { AIngle::Schema->parse($s) } "AIngle::Schema::Error::Parse",
        "size must be a POSITIVE int > 0";

    $s = <<EOJ;
{ "type": "fixed", "name": "somefixed", "size": 0.2 }
EOJ
    throws_ok { AIngle::Schema->parse($s) } "AIngle::Schema::Error::Parse",
        "size must be an int";

    $s = <<EOJ;
{ "type": "fixed", "name": "somefixed", "size": 5e2 }
EOJ
    my $schema = AIngle::Schema->parse($s);

    is $schema->type, 'fixed', "fixed";
    is $schema->fullname, 'somefixed', "name";
    is $schema->size, 500, "size of fixed";
    my $string = $schema->to_string;
    my $s2 = AIngle::Schema->parse($string);
    is_deeply $s2, $schema, "reserialized identically";
}
    
# fixed type referenced using short name without namespace
{
    my $s = <<EOJ;
{
  "type": "record",
  "name": "HandshakeRequest", "namespace":"org.apache.aingle.ipc",
  "fields": [
    {"name": "clientHash",
     "type": {"type": "fixed", "name": "MD5", "size": 16}},
    {"name": "clientProtocol", "type": ["null", "string"]},
    {"name": "serverHash", "type": "MD5"},
    {"name": "meta", "type": ["null", {"type": "map", "values": "bytes"}]}
  ]
}
EOJ
    my $schema = AIngle::Schema->parse($s);
    
    is $schema->type, 'record', 'HandshakeRequest type ok';
    is $schema->namespace, 'org.apache.aingle.ipc', 'HandshakeRequest namespace ok';
    is $schema->fields->[0]->{type}->{name}, 'MD5', 'HandshakeRequest clientHash type ok';
    is $schema->fields->[2]->{type}->{name}, 'MD5', 'HandshakeRequest serverHash type ok';
}

## Schema resolution
{
    my @s = split /\n/, <<EOJ;
{ "type": "int" }
{ "type": "long" }
{ "type": "float" }
{ "type": "double" }
{ "type": "boolean" }
{ "type": "null" }
{ "type": "string" }
{ "type": "bytes" }
{ "type": "array", "items": "string" }
{ "type": "fixed", "size": 1, "name": "fixed" }
{ "type": "enum", "name": "enum", "symbols": [ "s" ] }
{ "type": "map", "values": "long" }
{ "type": "record", "name": "r", "fields": [ { "name": "a", "type": "long" }] }
EOJ
    my %s;
    for (@s) {
        my $schema = AIngle::Schema->parse($_);
        $s{ $schema->type } = $schema;
        ok ( AIngle::Schema->match(
                reader => $schema,
                writer => $schema,
        ), "identical match!");
    }

    ## schema promotion
    match_ok($s{int},    $s{long});
    match_ok($s{int},    $s{float});
    match_ok($s{int},    $s{double});
    match_ok($s{long},   $s{float});
    match_ok($s{double}, $s{double});
    match_ok($s{float},  $s{double});

    ## some non promotion
    match_nok($s{long},    $s{int});
    match_nok($s{float},   $s{int});
    match_nok($s{string},  $s{bytes});
    match_nok($s{bytes},   $s{string});
    match_nok($s{double},  $s{float});
    match_nok($s{null},    $s{boolean});
    match_nok($s{boolean}, $s{int});
    match_nok($s{boolean}, $s{string});
    match_nok($s{boolean}, $s{fixed});

    ## complex type details
    my @alt = split /\n/, <<EOJ;
{ "type": "array", "items": "int" }
{ "type": "fixed", "size": 2, "name": "fixed" }
{ "type": "enum", "name": "enum2", "symbols": [ "b" ] }
{ "type": "map", "values": "null" }
{ "type": "record", "name": "r2", "fields": [ { "name": "b", "type": "long" }] }
EOJ
    my %alt;
    for (@alt) {
        my $schema = AIngle::Schema->parse($_);
        $alt{ $schema->type } = $schema;
        match_nok($s{$schema->type}, $schema, "not same subtypes/names");
    }
}

## union in a record.field
{
    my $s = AIngle::Schema::Record->new(
        struct => {
            name => 'saucisson',
            fields => [
                { name => 'a', type => [ 'long', 'null' ] },
            ],
        },
    );
    isa_ok $s, 'AIngle::Schema::Record';
    is $s->fields->[0]{name}, 'a', 'a';
    isa_ok $s->fields->[0]{type}, 'AIngle::Schema::Union';
}

sub match_ok {
    my ($w, $r, $msg) = @_;
    $msg ||= "match_ok";
    ok(AIngle::Schema->match(reader => $r, writer => $w), $msg);
}

sub match_nok {
    my ($w, $r, $msg) = @_;
    $msg ||= "non matching";
    ok !AIngle::Schema->match(reader => $r, writer => $w), $msg;
}

done_testing;
