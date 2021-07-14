

use strict;
use warnings;

use Test::More;
use File::Basename qw(basename);
use IO::File;
use_ok 'AIngle::DataFile';
use_ok 'AIngle::DataFileReader';

for my $path (glob '../../build/interop/data/*.aingle') {
    my $fn = basename($path);
    substr($fn, rindex $fn, '.') = '';
    my $idx = rindex $fn, '_';
    if (-1 < $idx) {
        my $codec = substr $fn, $idx + 1;
        next unless $AIngle::DataFile::ValidCodec{$codec};
    }
    my $fh = IO::File->new($path);
    AIngle::DataFileReader->new(fh => $fh);
    note("Succeeded to read ${path}");
}

done_testing;
