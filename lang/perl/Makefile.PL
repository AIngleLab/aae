

use Config;
use inc::Module::Install;

my $version;
for ('VERSION.txt', '../../share/VERSION.txt') {
    if (-f) {
        $version = `cat $_`;
        last;
    }
}
chomp $version;

license 'apache';
version $version;
readme_from 'lib/AIngle.pm';
all_from 'lib/AIngle.pm';
build_requires 'Test::More', 0.88;
test_requires 'Math::BigInt';
test_requires 'Perl::Critic';
test_requires 'Test::Exception';
requires 'Compress::Zlib';
requires 'Compress::Zstd';
requires 'Encode';
requires 'Error::Simple';
requires 'IO::String';
requires 'JSON::XS';
requires 'Object::Tiny';
requires 'Regexp::Common';
requires 'Try::Tiny';
requires 'parent';
unless ($Config{use64bitint}) {
    requires 'Math::BigInt';
}
auto_set_repository();

my %packages = (
    'AIngle'                    => 'lib/AIngle.pm',
    'AIngle::BinaryDecoder'     => 'lib/AIngle/BinaryDecoder.pm',
    'AIngle::BinaryEncoder'     => 'lib/AIngle/BinaryEncoder.pm',
    'AIngle::DataFile'          => 'lib/AIngle/DataFile.pm',
    'AIngle::DataFileReader'    => 'lib/AIngle/DataFileReader.pm',
    'AIngle::DataFileWriter'    => 'lib/AIngle/DataFileWriter.pm',
    'AIngle::Protocol'          => 'lib/AIngle/Protocol.pm',
    'AIngle::Protocol::Message' => 'lib/AIngle/Protocol/Message.pm',
    'AIngle::Schema'            => 'lib/AIngle/Schema.pm',
);
my %provides = map { $_ => { file => $packages{$_}, version => $version } } keys %packages;
provides(%provides);

WriteMakefile(PM_FILTER => "sed -e 's/\+\+MODULE_VERSION\+\+/$version/'");
