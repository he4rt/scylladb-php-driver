<?php

use Doctum\Doctum;
use Symfony\Component\Finder\Finder;

$envBaseUrl = match (getenv('DOCS_ENV')) {
    'local' => 'http://localhost:8000/api/',
    'production' => 'https://he4rt.github.io/scylladb-php-driver/api/',
    default => 'http://localhost:8000/api/',
};

$dir = '/stubs';
$iterator = Finder::create()
    ->files()
    ->name('*.php')
    ->in(__DIR__ . $dir);

echo "Building API documentation for ScyllaDB PHP Driver\n";
echo "Base URL: $envBaseUrl\n";

return new Doctum($iterator, [
    'title'                => 'ScyllaDB PHP Driver',
    'language'             => 'en', // Could be 'fr'
    'base_url'             => $envBaseUrl,
    'build_dir'            => __DIR__ . '/build/api',
    'cache_dir'            => __DIR__ . '/.cache/api',
    'source_dir'           => dirname($dir) . '/',
]);