

package AIngle::DataFile;
use strict;
use warnings;

use constant AINGLE_MAGIC => "Obj\x01";

use AIngle::Schema;

our $VERSION = '++MODULE_VERSION++';

our $HEADER_SCHEMA = AIngle::Schema->parse(<<EOH);
{"type": "record", "name": "org.apache.aingle.file.Header",
  "fields" : [
    {"name": "magic", "type": {"type": "fixed", "name": "Magic", "size": 4}},
    {"name": "meta", "type": {"type": "map", "values": "bytes"}},
    {"name": "sync", "type": {"type": "fixed", "name": "Sync", "size": 16}}
  ]
}
EOH

our %ValidCodec = (
    null      => 1,
    deflate   => 1,
    bzip2     => 1,
    zstandard => 1,
);

sub is_codec_valid {
    my $datafile = shift;
    my $codec = shift || '';
    return $ValidCodec{$codec};
}

+1;
