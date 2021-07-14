<?php
/**

 */

namespace Apache\AIngle\Tests;

use Apache\AIngle\DataFile\AIngleDataIO;
use Apache\AIngle\IO\AIngleFile;
use Apache\AIngle\Schema\AIngleSchema;
use PHPUnit\Framework\TestCase;

class InterOpTest extends TestCase
{
    private $projection_json;
    private $projection;

    public function setUp(): void
    {
        $interop_schema_file_name = AINGLE_INTEROP_SCHEMA;
        $this->projection_json = file_get_contents($interop_schema_file_name);
        $this->projection = AIngleSchema::parse($this->projection_json);
    }

    public function file_name_provider()
    {
        $data_dir = AINGLE_BUILD_DATA_DIR;
        $data_files = array();
        if (!($dh = opendir($data_dir))) {
            die("Could not open data dir '$data_dir'\n");
        }

        while ($file = readdir($dh)) {
            if (preg_match('/^[a-z]+(_deflate|_snappy|_zstandard|_bzip2)?\.aingle$/', $file)) {
                $data_files [] = implode(DIRECTORY_SEPARATOR, array($data_dir, $file));
            } else if (preg_match('/[^.]/', $file)) {
                echo "Skipped: $data_dir/$file", PHP_EOL;
            }
        }
        closedir($dh);

        $ary = array();
        foreach ($data_files as $df) {
            echo "Reading: $df", PHP_EOL;
            $ary [] = array($df);
        }
        return $ary;
    }

    /**
     * @coversNothing
     * @dataProvider file_name_provider
     */
    public function test_read($file_name)
    {
        $dr = AIngleDataIO::openFile(
            $file_name, AIngleFile::READ_MODE, $this->projection_json);

        $data = $dr->data();

        $this->assertNotCount(0, $data, sprintf("no data read from %s", $file_name));

        foreach ($data as $idx => $datum) {
            $this->assertNotNull($datum, sprintf("null datum from %s", $file_name));
        }
    }
}
