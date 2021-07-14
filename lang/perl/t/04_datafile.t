

#!/usr/bin/env perl

use strict;
use warnings;
use File::Temp;
use AIngle::DataFile;
use AIngle::BinaryEncoder;
use AIngle::BinaryDecoder;
use AIngle::Schema;
use Test::Exception;
use Test::More;

use_ok 'AIngle::DataFileReader';
use_ok 'AIngle::DataFileWriter';

my $tmpfh = File::Temp->new(UNLINK => 1);

my $schema = AIngle::Schema->parse(<<EOP);
{ "type": "map", "values": { "type": "array", "items": "string" } }
EOP

my $write_file = AIngle::DataFileWriter->new(
    fh            => $tmpfh,
    writer_schema => $schema,
    metadata      => {
        some => 'metadata',
    },
);

my $data = {
    a => [ "2.2", "4.4" ],
    b => [ "2.4", "2", "-4", "4", "5" ],
    c => [ "0" ],
};

$write_file->print($data);
$write_file->flush;

## rewind
seek $tmpfh, 0, 0;
my $uncompressed_size = -s $tmpfh;

my $read_file = AIngle::DataFileReader->new(
    fh            => $tmpfh,
    reader_schema => $schema,
);
is $read_file->metadata->{'aingle.codec'}, 'null', 'aingle.codec';
is $read_file->metadata->{'some'}, 'metadata', 'custom meta';

my @all = $read_file->all;
is scalar @all, 1, "one object back";
is_deeply $all[0], $data, "Our data is intact!";


## codec tests
{
    throws_ok {
        AIngle::DataFileWriter->new(
            fh            => File::Temp->new,
            writer_schema => $schema,
            codec         => 'unknown',
        );
    } "AIngle::DataFile::Error::InvalidCodec", "invalid codec";

    ## rewind
    seek $tmpfh, 0, 0;
    local $AIngle::DataFile::ValidCodec{null} = 0;
    $read_file = AIngle::DataFileReader->new(
        fh            => $tmpfh,
        reader_schema => $schema,
    );

    throws_ok {
        $read_file->all;
    } "AIngle::DataFile::Error::UnsupportedCodec", "I've removed 'null' :)";

    ## deflate!
    my $zfh = File::Temp->new(UNLINK => 0);
    my $write_file = AIngle::DataFileWriter->new(
        fh            => $zfh,
        writer_schema => $schema,
        codec         => 'deflate',
        metadata      => {
            some => 'metadata',
        },
    );
    $write_file->print($data);
    $write_file->flush;

    ## rewind
    seek $zfh, 0, 0;

    my $read_file = AIngle::DataFileReader->new(
        fh            => $zfh,
        reader_schema => $schema,
    );
    is $read_file->metadata->{'aingle.codec'}, 'deflate', 'aingle.codec';
    is $read_file->metadata->{'some'}, 'metadata', 'custom meta';

    my @all = $read_file->all;
    is scalar @all, 1, "one object back";
    is_deeply $all[0], $data, "Our data is intact!";


    ## bzip2!
    $zfh = File::Temp->new(UNLINK => 0);
    $write_file = AIngle::DataFileWriter->new(
        fh            => $zfh,
        writer_schema => $schema,
        codec         => 'bzip2',
        metadata      => {
            some => 'metadata',
        },
    );
    $write_file->print($data);
    $write_file->flush;

    ## rewind
    seek $zfh, 0, 0;

    $read_file = AIngle::DataFileReader->new(
        fh            => $zfh,
        reader_schema => $schema,
    );
    is $read_file->metadata->{'aingle.codec'}, 'bzip2', 'aingle.codec';
    is $read_file->metadata->{'some'}, 'metadata', 'custom meta';

    @all = $read_file->all;
    is scalar @all, 1, "one object back";
    is_deeply $all[0], $data, "Our data is intact!";


    ## zstandard!
    $zfh = File::Temp->new(UNLINK => 0);
    $write_file = AIngle::DataFileWriter->new(
        fh            => $zfh,
        writer_schema => $schema,
        codec         => 'zstandard',
        metadata      => {
            some => 'metadata',
        },
    );
    $write_file->print($data);
    $write_file->flush;

    ## rewind
    seek $zfh, 0, 0;

    $read_file = AIngle::DataFileReader->new(
        fh            => $zfh,
        reader_schema => $schema,
    );
    is $read_file->metadata->{'aingle.codec'}, 'zstandard', 'aingle.codec';
    is $read_file->metadata->{'some'}, 'metadata', 'custom meta';

    @all = $read_file->all;
    is scalar @all, 1, "one object back";
    is_deeply $all[0], $data, "Our data is intact!";
}

done_testing;
