

package AIngle::Protocol;
use strict;
use warnings;

use Carp;
use JSON::XS();
use Try::Tiny;
use AIngle::Protocol::Message;
use AIngle::Schema;
use Error;
use Object::Tiny qw{
    name
    namespace
    doc
    types
    messages
};

our $VERSION = '++MODULE_VERSION++';

my $json = JSON::XS->new->allow_nonref;

sub parse {
    my $class     = shift;
    my $enc_proto = shift
        or throw AIngle::Protocol::Error::Parse("protocol cannot be empty");

    my $struct = try {
        $json->decode($enc_proto);
    }
    catch {
        throw AIngle::Protocol::Error::Parse(
            "Cannot parse json string: $_"
        );
    };
    return $class->from_struct($struct);
}

sub from_struct {
    my $class = shift;
    my $struct = shift || {};
    my $name = $struct->{protocol};
    unless (defined $name or length $name) {
        throw AIngle::Protocol::Error::Parse("protocol name is required");
    }

    my $types = $class->parse_types($struct->{types});

    my $messages = $struct->{messages} ?
        $class->parse_messages($struct->{messages}, $types) : undef;

    my $protocol = $class->SUPER::new(
        name      => $name,
        namespace => $struct->{namespace},
        doc       => $struct->{doc},
        types     => $types,
        messages  => $messages,
    );
    return $protocol;
}

sub parse_types {
    my $class = shift;
    my $types = shift || [];

    my %types;
    my $names = {};
    for (@$types) {
        try {
            my $schema = AIngle::Schema->parse_struct($_, $names);
            $types{ $schema->fullname } = $schema;
        }
        catch {
            throw AIngle::Protocol::Error::Parse("errors in parsing types: $_");
        };
    }
    return \%types;
}

sub parse_messages {
    my $class = shift;
    my $messages = shift || {};
    my $types = shift;
    my $m = {};
    for my $name (keys %$messages) {
        $m->{$name} = AIngle::Protocol::Message->new($messages->{$name}, $types);
    }
    return $m;
}

sub fullname {
    my $protocol = shift;
    return join ".", grep { $_ } map { $protocol->$_ } qw{ namespace name };
}

package AIngle::Protocol::Error::Parse;
use parent 'Error::Simple';

1;
