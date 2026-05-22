# gen_arginfo.sh

Small wrapper around PHP's official `build/gen_stub.php`. CMake's
`cmake/GenStubs.cmake` locates `gen_stub.php` from the active PHP install
(`${PHP_PREFIX}/lib/php/build/gen_stub.php` typically) and invokes this
wrapper to generate `*_arginfo.h` next to each `*.stub.php` at build time.

Two project-specific fixups happen here, not in a vendored gen_stub fork:

1. **Pre-process:** strip `declare(strict_types=1);` from a tmp copy of the
   stub. Upstream gen_stub errors on it ("Unexpected node Stmt_Declare") but
   the declare has no effect on arginfo output. Candidate for upstreaming.

2. **Post-process:** inject `(zend_function *)` cast on the emitted
   `zend_add_function_attribute(zend_hash_str_find_ptr(...))` /
   `zend_add_parameter_attribute(zend_hash_str_find_ptr(...))` calls.
   `zend_hash_str_find_ptr` returns `void*`; the implicit conversion is
   fine in C but C++ requires an explicit cast. Candidate for upstreaming.

If `gen_stub.php` isn't found in the active PHP install, install the PHP
development build tools (e.g. `php-dev` on Debian/Ubuntu, `php-devel` on
RHEL) or pass `-DPHP_DRIVER_GEN_STUB_SCRIPT=/abs/path/gen_stub.php` to
CMake.
