<?php
/**

 */

if (file_exists(__DIR__ . '/../vendor/autoload.php')) {
    include __DIR__ . '/../vendor/autoload.php';
} else {
    include __DIR__ . '/../lib/autoload.php';
}

define('AINGLE_TEST_HELPER_DIR', __DIR__);

define('TEST_TEMP_DIR', implode(DIRECTORY_SEPARATOR, [AINGLE_TEST_HELPER_DIR, 'tmp']));

define('AINGLE_BASE_DIR', dirname(AINGLE_TEST_HELPER_DIR, 3));
define('AINGLE_SHARE_DIR', implode(DIRECTORY_SEPARATOR, [AINGLE_BASE_DIR, 'share']));
define('AINGLE_BUILD_DIR', implode(DIRECTORY_SEPARATOR, [AINGLE_BASE_DIR, 'build']));
define('AINGLE_BUILD_DATA_DIR', implode(DIRECTORY_SEPARATOR, [AINGLE_BUILD_DIR, 'interop', 'data']));
define('AINGLE_TEST_SCHEMAS_DIR', implode(DIRECTORY_SEPARATOR, [AINGLE_SHARE_DIR, 'test', 'schemas']));
define('AINGLE_INTEROP_SCHEMA', implode(DIRECTORY_SEPARATOR, [AINGLE_TEST_SCHEMAS_DIR, 'interop.ain']));
