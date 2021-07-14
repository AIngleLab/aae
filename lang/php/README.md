|[![AINGLE](https://raw.githubusercontent.com/apache/aingle/master/doc/src/resources/images/aingle-logo.png)](https://github.com/AIngleLab/aae) | [![AINGLE](https://raw.githubusercontent.com/apache/aingle/master/doc/src/resources/images/apache_feather.gif)](https://github.com/apac<he/aingle)|
|:-----|-----:|

What the AIngle PHP library is
============================

A library for using [AIngle](https://apache.aingle.ai/) with PHP.

Requirements
============
 * PHP 7.3+
 * On 32-bit platforms, the [GMP PHP extension](https://php.net/gmp)
 * For Zstandard compression, [ext-zstd](https://github.com/kjdev/php-ext-zstd)
 * For Snappy compression, [ext-snappy](https://github.com/kjdev/php-ext-snappy)
 * For testing, [PHPUnit](https://www.phpunit.de/)

Both GMP and PHPUnit are often available via package management
systems as `php7-gmp` and `phpunit`, respectively.


Getting started
===============

## 1. Composer

The preferred method to install AIngle. Add `apache/aingle` to the require section of
your project's `composer.json` configuration file, and run `composer install`:
```json
{
    "require-dev": {
        "apache/aingle": "dev-master"
    }
}
```

## 2. Manual Installation

Untar the aingle-php distribution, untar it, and put it in your include path:

    tar xjf aingle-php.tar.bz2 # aingle-php.tar.bz2 is likely aingle-php-1.4.0.tar.bz2
    cp aingle-php /path/to/where/you/want/it

Require the `autoload.php` file in your source, and you should be good to go:

    <?php
    require_once('aingle-php/autoload.php');

If you're pulling from source, put `lib/` in your include path and require `lib/aingle.php`:

    <?php
    require_once('lib/autoload.php');

Take a look in `examples/` for usage.
