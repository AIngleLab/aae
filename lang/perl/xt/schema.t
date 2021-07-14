

use strict;
use warnings;

use Test::More;
use File::Find;
use_ok 'AIngle::Schema';

sub parse {
    next unless /\.ain$/;
    open(my $fh, '<', $_);
    local $/ = undef;
    my $schema = <$fh>;
    close $fh;
    AIngle::Schema->parse($schema);
    note("Successfully parsed: $_");
}

# Ensure that all schema files under the "share" directory can be parsed
{
    find(\&parse, '../../share');
}

done_testing;
