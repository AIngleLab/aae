

package AIngle::Protocol::Message;

use strict;
use warnings;

use AIngle::Schema;
use AIngle::Protocol;
use Error;

use Object::Tiny qw{
    doc
    request
    response
    errors
};

our $VERSION = '++MODULE_VERSION++';

sub new {
    my $class = shift;
    my $struct = shift;
    my $types = shift;

    my $resp_struct = $struct->{response}
        or throw AIngle::Protocol::Error::Parse("response is missing");

    my $req_struct = $struct->{request}
        or throw AIngle::Protocol::Error::Parse("request is missing");

    my $request = [
        map { AIngle::Schema::Field->new($_, $types) } @$req_struct
    ];

    my $err_struct = $struct->{errors};

    my $response = AIngle::Schema->parse_struct($resp_struct, $types);
    my $errors = $err_struct ? AIngle::Schema->parse_struct($err_struct, $types) : undef;

    return $class->SUPER::new(
        doc      => $struct->{doc},
        request  => $request,
        response => $response,
        errors   => $errors,
    );

}

1;
