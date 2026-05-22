# gen_stub

Vendored copy of PHP's `gen_stub.php` (sourced from PHP 8.5's `lib/php/build/gen_stub.php`).

Used at CMake configure time to generate `_arginfo.h` headers from `.stub.php`
files. PHP-Parser is auto-downloaded on first run into
`tools/gen_stub/PHP-Parser-<version>/` (gitignored).

The generated `_arginfo.h` files live in the CMake binary dir; they are no
longer checked into the repo.

To regenerate manually (e.g. after editing a stub):
```
cmake --build out/<preset> --target regen-stubs
```
