

#!/usr/bin/env perl

use strict;
use warnings;
use AIngle::Schema;
use Config;
use Test::More;
use Test::Exception;
use Math::BigInt;

use_ok 'AIngle::BinaryEncoder';

sub primitive_ok {
    my ($primitive_type, $primitive_val, $expected_enc) = @_;

    my $data;
    my $meth = "encode_$primitive_type";
    AIngle::BinaryEncoder->$meth(
        undef, $primitive_val, sub { $data = ${$_[0]} }
    );
    is $data, $expected_enc, "primitive $primitive_type encoded correctly";
    return $data;
}

## some primitive testing
{
    primitive_ok null    =>    undef, '';
    primitive_ok null    => 'whatev', '';

    primitive_ok boolean => 0, "\x0";
    primitive_ok boolean => 1, "\x1";

    ## - high-bit of each byte should be set except for last one
    ## - rest of bits are:
    ## - little endian
    ## - zigzag coded
    primitive_ok long    =>        0, pack("C*", 0);
    primitive_ok long    =>        1, pack("C*", 0x2);
    primitive_ok long    =>       -1, pack("C*", 0x1);
    primitive_ok int     =>       -1, pack("C*", 0x1);
    primitive_ok int     =>      -20, pack("C*", 0b0010_0111);
    primitive_ok int     =>       20, pack("C*", 0b0010_1000);
    primitive_ok int     =>       63, pack("C*", 0b0111_1110);
    primitive_ok int     =>       64, pack("C*", 0b1000_0000, 0b0000_0001);
    my $p =
    primitive_ok int     =>      -65, pack("C*", 0b1000_0001, 0b0000_0001);
    primitive_ok int     =>       65, pack("C*", 0b1000_0010, 0b0000_0001);
    primitive_ok int     =>       99, "\xc6\x01";

    ## BigInt values still work
    primitive_ok int     => Math::BigInt->new(-65), $p;

    throws_ok {
        my $toobig;
        if ($Config{use64bitint}) {
            $toobig = 1<<32;
        }
        else {
            require Math::BigInt;
            $toobig = Math::BigInt->new(1)->blsft(32);
        }
        primitive_ok int => $toobig, undef;
    } "AIngle::BinaryEncoder::Error", "33 bits";

    throws_ok {
        primitive_ok int => Math::BigInt->new(1)->blsft(63), undef;
    } "AIngle::BinaryEncoder::Error", "65 bits";

    for (qw(long int)) {
        throws_ok {
            primitive_ok $_ =>  "x", undef;
        } 'AIngle::BinaryEncoder::Error', 'numeric values only';
    };
    # In Unicode, there are decimals that aren't 0-9.
    # Make sure we handle non-ascii decimals cleanly.
    for (qw(long int)) {
        throws_ok {
            primitive_ok $_ =>  "\N{U+0661}", undef;
        } 'AIngle::BinaryEncoder::Error', 'ascii decimals only';
    };
}

## spec examples
{
    my $enc = '';
    my $schema = AIngle::Schema->parse(q({ "type": "string" }));
    AIngle::BinaryEncoder->encode(
        schema => $schema,
        data => "foo",
        emit_cb => sub { $enc .= ${ $_[0] } },
    );
    is $enc, "\x06\x66\x6f\x6f", "Binary_Encodings.Primitive_Types";

    $schema = AIngle::Schema->parse(<<EOJ);
          {
          "type": "record",
          "name": "test",
          "fields" : [
          {"name": "a", "type": "long"},
          {"name": "b", "type": "string"}
          ]
          }
EOJ
    $enc = '';
    AIngle::BinaryEncoder->encode(
        schema => $schema,
        data => { a => 27, b => 'foo' },
        emit_cb => sub { $enc .= ${ $_[0] } },
    );
    is $enc, "\x36\x06\x66\x6f\x6f", "Binary_Encodings.Complex_Types.Records";

    $enc = '';
    $schema = AIngle::Schema->parse(q({"type": "array", "items": "long"}));
    AIngle::BinaryEncoder->encode(
        schema => $schema,
        data => [3, 27],
        emit_cb => sub { $enc .= ${ $_[0] } },
    );
    is $enc, "\x04\x06\x36\x00", "Binary_Encodings.Complex_Types.Arrays";

    $enc = '';
    $schema = AIngle::Schema->parse(q(["string","null"]));
    AIngle::BinaryEncoder->encode(
        schema => $schema,
        data => undef,
        emit_cb => sub { $enc .= ${ $_[0] } },
    );
    is $enc, "\x02", "Binary_Encodings.Complex_Types.Unions-null";

    $enc = '';
    AIngle::BinaryEncoder->encode(
        schema => $schema,
        data => "a",
        emit_cb => sub { $enc .= ${ $_[0] } },
    );
    is $enc, "\x00\x02\x61", "Binary_Encodings.Complex_Types.Unions-a";
}

done_testing;
