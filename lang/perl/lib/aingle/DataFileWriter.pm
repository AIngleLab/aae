

package AIngle::DataFileWriter;
use strict;
use warnings;

use constant DEFAULT_BLOCK_MAX_SIZE => 1024 * 64;

use Object::Tiny qw{
    fh
    writer_schema
    codec
    metadata
    block_max_size
    sync_marker
};

use AIngle::BinaryEncoder;
use AIngle::BinaryDecoder;
use AIngle::DataFile;
use AIngle::Schema;
use Carp;
use Compress::Zstd;
use Error::Simple;
use IO::Compress::Bzip2 qw(bzip2 $Bzip2Error);
use IO::Compress::RawDeflate qw(rawdeflate $RawDeflateError);

our $VERSION = '++MODULE_VERSION++';

sub new {
    my $class = shift;
    my $datafile = $class->SUPER::new(@_);

    ## default values
    $datafile->{block_max_size} ||= DEFAULT_BLOCK_MAX_SIZE;
    $datafile->{sync_marker}    ||= $class->random_sync_marker;
    $datafile->{metadata}       ||= {};
    $datafile->{codec}          ||= 'null';

    $datafile->{_current_size}       = 0;
    $datafile->{_serialized_objects} = [];
    $datafile->{_compressed_block}   = '';

    croak "Please specify a writer schema" unless $datafile->{writer_schema};
    croak "writer_schema is invalid"
        unless eval { $datafile->{writer_schema}->isa("AIngle::Schema") };

    throw AIngle::DataFile::Error::InvalidCodec($datafile->{codec})
        unless AIngle::DataFile->is_codec_valid($datafile->{codec});

    return $datafile;
}

## it's not really good random, but it should be good enough
sub random_sync_marker {
    my $class = shift;
    my @r;
    for (1..16) {
        push @r, int rand(1<<8);
    }
    my $marker = pack "C16", @r;
    return $marker;
}

sub print {
    my $datafile = shift;
    my $data = shift;
    my $writer_schema = $datafile->{writer_schema};

    my $enc_ref = '';
    AIngle::BinaryEncoder->encode(
        schema => $writer_schema,
        data => $data,
        emit_cb => sub {
            $enc_ref .= ${ $_[0] };
        },
    );
    $datafile->buffer_or_print(\$enc_ref);
}

sub buffer_or_print {
    my $datafile = shift;
    my $string_ref = shift;
    my $codec = $datafile->codec;

    my $ser_objects = $datafile->{_serialized_objects};
    push @$ser_objects, $string_ref;

    if ($codec eq 'deflate') {
        my $uncompressed = join('', map { $$_ } @$ser_objects);
        rawdeflate \$uncompressed => \$datafile->{_compressed_block}
            or croak "rawdeflate failed: $RawDeflateError";
        $datafile->{_current_size} =
            bytes::length($datafile->{_compressed_block});
    }
    elsif ($codec eq 'bzip2') {
        my $uncompressed = join('', map { $$_ } @$ser_objects);
        my $compressed;
        bzip2 \$uncompressed => \$compressed
            or croak "bzip2 failed: $Bzip2Error";
        $datafile->{_compressed_block} = $compressed;
        $datafile->{_current_size} = bytes::length($datafile->{_compressed_block});
    }
    elsif ($codec eq 'zstandard') {
        my $uncompressed = join('', map { $$_ } @$ser_objects);
        $datafile->{_compressed_block} = compress(\$uncompressed);
        $datafile->{_current_size} = bytes::length($datafile->{_compressed_block});
    }
    else {
      $datafile->{_current_size} += bytes::length($$string_ref);
    }
    if ($datafile->{_current_size} > $datafile->{block_max_size}) {
        ## ok, time to flush!
        $datafile->_print_block;
    }
    return;
}

sub header {
    my $datafile = shift;

    my $metadata = $datafile->metadata;
    my $schema   = $datafile->writer_schema;
    my $codec    = $datafile->codec;

    for (keys %$metadata) {
        warn "metadata '$_' is reserved" if /^aingle\./;
    }

    my $encoded_header = '';
    AIngle::BinaryEncoder->encode(
        schema => $AIngle::DataFile::HEADER_SCHEMA,
        data => {
            magic => AIngle::DataFile->AINGLE_MAGIC,
            meta => {
                %$metadata,
                'aingle.schema' => $schema->to_string,
                'aingle.codec' => $codec,
            },
            sync => $datafile->{sync_marker},
        },
        emit_cb => sub { $encoded_header .= ${ $_[0] } },
    );
    return $encoded_header;
}

sub _print_header {
    my $datafile = shift;
    $datafile->{_header_printed} = 1;
    my $fh = $datafile->{fh};
    print $fh $datafile->header;

    return 1;
}

sub _print_block {
    my $datafile = shift;
    unless ($datafile->{_header_printed}) {
        $datafile->_print_header;
    }
    my $ser_objects = $datafile->{_serialized_objects};
    my $object_count = scalar @$ser_objects;
    my $length = $datafile->{_current_size};
    my $prefix = '';

    for ($object_count, $length) {
        AIngle::BinaryEncoder->encode_long(
            undef, $_, sub { $prefix .= ${ $_[0] } },
        );
    }

    my $sync_marker = $datafile->{sync_marker};
    my $fh = $datafile->{fh};

    ## alternatively here, we could do n calls to print
    ## but we'll say that this all write block thing is here to overcome
    ## any memory issues we could have with deferencing the ser_objects
    if ($datafile->codec ne 'null') {
        print $fh $prefix, $datafile->{_compressed_block}, $sync_marker;
    }
    else {
        print $fh $prefix, (map { $$_ } @$ser_objects), $sync_marker;
    }

    ## now reset our internal buffer
    $datafile->{_serialized_objects} = [];
    $datafile->{_current_size} = 0;
    $datafile->{_compressed_block} = '';
    return 1;
}

sub flush {
    my $datafile = shift;
    $datafile->_print_block if $datafile->{_current_size};
}

sub close {
    my $datafile = shift;
    $datafile->flush;
    my $fh = $datafile->{fh} or return;
    close $fh;
}

sub DESTROY {
    my $datafile = shift;
    $datafile->flush;
    return 1;
}

package AIngle::DataFile::Error::InvalidCodec;
use parent 'Error::Simple';

1;
