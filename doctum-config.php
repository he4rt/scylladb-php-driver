<?php

use Doctum\Doctum;
use Symfony\Component\Finder\Finder;

$dir = '/stubs';
$iterator = Finder::create()
    ->files()
    ->name('*.php')
    ->in(__DIR__ . $dir);

return new Doctum($iterator, [
    'title'                => 'ScyllaDB PHP Driver',
    'language'             => 'en', // Could be 'fr'
    'base_url'             => 'http://localhost:8000/api/',
    'build_dir'            => __DIR__ . '/build/api',
    'cache_dir'            => __DIR__ . '/cache/api',
    'source_dir'           => dirname($dir) . '/',
]);