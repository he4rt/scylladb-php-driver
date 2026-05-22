<?php
/**
 * Generates `<basename>_descriptor.cpp` from a `.stub.php` file.
 *
 * What it produces:
 *   - the `php_scylladb_<snake>_ce` global
 *   - the `zend_object_handlers php_scylladb_<snake>_handlers` global
 *   - a register fn that calls register_class_*() with deps wired from
 *     extends/implements, applies create_object + handler overrides via
 *     weakly-declared callbacks, and calls a weak post_register hook
 *   - the PHP_SCYLLADB_REGISTER_CLASS / _DEPS macro invocation
 *
 * What the user's .cpp keeps:
 *   - ZEND_METHOD bodies
 *   - the `_new`, `_free`, `_properties`, `_compare`, `_gc`, `_cast`,
 *     `_hash_value`, `_clone`, `_post_register` callbacks (named by
 *     convention; defined only as needed — weak refs leave them NULL
 *     when not implemented)
 *   - any custom API functions (e.g. `_instantiate`)
 *
 * Opt-out:
 *   - A class with `@scylladb-no-generate` in its docblock is skipped
 *     entirely. Use this for unusual hierarchies (e.g. exception batch).
 *
 * Usage:
 *   php gen_class_descriptor.php <stub.php> <out.cpp> <arginfo_header>
 */

declare(strict_types=1);

if ($argc !== 4) {
    fwrite(STDERR, "usage: gen_class_descriptor.php <stub.php> <out.cpp> <arginfo_header>\n");
    exit(2);
}

[$_, $stubPath, $outPath, $arginfoHeader] = $argv;

if (!is_file($stubPath)) {
    fwrite(STDERR, "stub not found: $stubPath\n");
    exit(1);
}

$src = file_get_contents($stubPath);
$classes = parse_stub($src);
if (!$classes) {
    // No classes (rare — e.g. a stub with only function decls); emit empty file.
    file_put_contents($outPath, "/* generated: no classes in stub */\n");
    exit(0);
}

$out = emit_descriptor_file($classes, $arginfoHeader, basename($stubPath));
$dir = dirname($outPath);
if (!is_dir($dir)) {
    mkdir($dir, 0755, true);
}
file_put_contents($outPath, $out);

// ─────────────────────────────────────────────────────────────────────────────
// Stub parser
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Returns array of class records. Each record:
 *   [
 *     'fqn'        => "Cassandra\\Foo\\Bar",   // backslashes single, will be doubled at emit
 *     'kind'       => 'class' | 'interface',
 *     'is_final'   => bool,
 *     'is_abstract'=> bool,
 *     'parents'    => array of ['fqn' => '...', 'kind' => 'extends'|'implements'],
 *     'no_generate'=> bool,
 *   ]
 */
function parse_stub(string $src): array
{
    $tokens = token_get_all($src);
    $out = [];
    $namespace = '';
    $pendingDoc = null;

    $n = count($tokens);
    for ($i = 0; $i < $n; $i++) {
        $t = $tokens[$i];
        if (is_string($t)) {
            continue;
        }
        [$id, $text] = [$t[0], $t[1]];

        if ($id === T_DOC_COMMENT) {
            $pendingDoc = $text;
            continue;
        }
        // Anything other than whitespace/comments resets the pending doc.
        if ($id !== T_WHITESPACE && $id !== T_COMMENT && $id !== T_DOC_COMMENT
            && $id !== T_DECLARE && $id !== T_USE && $id !== T_OPEN_TAG && $id !== T_NAMESPACE) {
            // Will be reassigned by class/interface case below if needed.
        }

        if ($id === T_NAMESPACE) {
            // namespace Foo\Bar { ... } or namespace Foo\Bar;
            $ns = '';
            $j = $i + 1;
            while ($j < $n) {
                $tt = $tokens[$j];
                if (is_array($tt) && ($tt[0] === T_STRING || $tt[0] === T_NS_SEPARATOR
                        || $tt[0] === T_NAME_QUALIFIED || $tt[0] === T_NAME_FULLY_QUALIFIED)) {
                    $ns .= $tt[1];
                    $j++;
                    continue;
                }
                if (is_array($tt) && $tt[0] === T_WHITESPACE) { $j++; continue; }
                break;
            }
            $namespace = trim($ns, '\\');
            $i = $j;
            continue;
        }

        if ($id === T_CLASS || $id === T_INTERFACE) {
            $kind = ($id === T_INTERFACE) ? 'interface' : 'class';

            // Walk back to find modifiers (final / abstract) and the doc comment.
            $isFinal = false;
            $isAbstract = false;
            $doc = $pendingDoc;
            for ($k = $i - 1; $k >= 0; $k--) {
                $pt = $tokens[$k];
                if (is_array($pt)) {
                    if ($pt[0] === T_FINAL)    { $isFinal = true;    continue; }
                    if ($pt[0] === T_ABSTRACT) { $isAbstract = true; continue; }
                    if ($pt[0] === T_WHITESPACE || $pt[0] === T_COMMENT) { continue; }
                    if ($pt[0] === T_DOC_COMMENT) { $doc = $pt[1]; break; }
                }
                break;
            }
            $pendingDoc = null;

            // Class name.
            $j = $i + 1;
            while ($j < $n && is_array($tokens[$j]) && $tokens[$j][0] === T_WHITESPACE) {
                $j++;
            }
            if ($j >= $n || !is_array($tokens[$j]) || $tokens[$j][0] !== T_STRING) {
                continue; // anonymous class or odd shape — skip
            }
            $name = $tokens[$j][1];

            // Walk forward collecting extends/implements until we hit '{'.
            $parents = [];
            $j++;
            $currentKind = null;
            $currentName = '';
            $flush = function () use (&$parents, &$currentKind, &$currentName, $namespace) {
                $nm = trim($currentName);
                if ($nm !== '' && $currentKind !== null) {
                    $parents[] = [
                        'fqn'  => normalize_parent_fqn($nm, $namespace),
                        'kind' => $currentKind,
                    ];
                }
                $currentName = '';
            };
            while ($j < $n) {
                $jt = $tokens[$j];
                if (is_string($jt)) {
                    if ($jt === '{') { $flush(); break; }
                    if ($jt === ',') { $flush(); $j++; continue; }
                    $j++;
                    continue;
                }
                $jid = $jt[0];
                if ($jid === T_EXTENDS)    { $flush(); $currentKind = 'extends';    $j++; continue; }
                if ($jid === T_IMPLEMENTS) { $flush(); $currentKind = 'implements'; $j++; continue; }
                if ($jid === T_STRING || $jid === T_NS_SEPARATOR
                    || $jid === T_NAME_QUALIFIED || $jid === T_NAME_FULLY_QUALIFIED) {
                    $currentName .= $jt[1];
                    $j++;
                    continue;
                }
                if ($jid === T_WHITESPACE) { $j++; continue; }
                $j++;
            }

            $fqn = $namespace === '' ? $name : "$namespace\\$name";
            $hasValueHandlers = $doc !== null && str_contains($doc, '@scylladb-value-handlers');
            $out[] = [
                'fqn'              => $fqn,
                'kind'             => $kind,
                'is_final'         => $isFinal,
                'is_abstract'      => $isAbstract,
                'parents'          => $parents,
                'no_generate'      => $doc !== null && (str_contains($doc, '@scylladb-no-generate')
                                                       || str_contains($doc, '@scylladb-skip')),
                'value_handlers'   => $hasValueHandlers,
            ];

            $i = $j;
        }
    }

    return $out;
}

function normalize_parent_fqn(string $raw, string $currentNamespace): string
{
    if (str_starts_with($raw, '\\')) {
        return ltrim($raw, '\\');
    }
    // Unqualified name inside a namespace — could refer to a class in the same
    // namespace (e.g. RetryPolicy from `namespace Cassandra { ... implements
    // RetryPolicy }`). PHP would resolve via use-statements; we don't track
    // those. Heuristic: if the unqualified token starts with a known PHP-core
    // / SPL name, treat as global; otherwise prefix with current namespace.
    if ($currentNamespace !== '' && !is_core_php_class($raw)) {
        return "$currentNamespace\\$raw";
    }
    return $raw;
}

function is_core_php_class(string $name): bool
{
    static $known = [
        'Iterator', 'IteratorAggregate', 'Traversable', 'Countable',
        'ArrayAccess', 'Stringable', 'Throwable',
        'Exception', 'Error',
        'RuntimeException', 'LogicException', 'DomainException',
        'InvalidArgumentException', 'RangeException',
    ];
    return in_array($name, $known, true);
}

// ─────────────────────────────────────────────────────────────────────────────
// Emitter
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Map an FQN-style parent to either:
 *   ['mode' => 'registry', 'fqn' => '...']               — registered via descriptor system
 *   ['mode' => 'extern',   'ce'  => 'zend_ce_xxx', 'include' => null|'header.h']
 */
function classify_parent(string $fqn): array
{
    static $extern = [
        'Iterator'                 => ['ce' => 'zend_ce_iterator',                    'include' => 'Zend/zend_interfaces.h'],
        'IteratorAggregate'        => ['ce' => 'zend_ce_aggregate',                   'include' => 'Zend/zend_interfaces.h'],
        'Traversable'              => ['ce' => 'zend_ce_traversable',                 'include' => 'Zend/zend_interfaces.h'],
        'Countable'                => ['ce' => 'zend_ce_countable',                   'include' => 'Zend/zend_interfaces.h'],
        'ArrayAccess'              => ['ce' => 'zend_ce_arrayaccess',                 'include' => 'Zend/zend_interfaces.h'],
        'Stringable'               => ['ce' => 'zend_ce_stringable',                  'include' => 'Zend/zend_interfaces.h'],
        'Throwable'                => ['ce' => 'zend_ce_throwable',                   'include' => 'Zend/zend_exceptions.h'],
        'RuntimeException'         => ['ce' => 'spl_ce_RuntimeException',             'include' => 'ext/spl/spl_exceptions.h'],
        'LogicException'           => ['ce' => 'spl_ce_LogicException',               'include' => 'ext/spl/spl_exceptions.h'],
        'DomainException'          => ['ce' => 'spl_ce_DomainException',              'include' => 'ext/spl/spl_exceptions.h'],
        'InvalidArgumentException' => ['ce' => 'spl_ce_InvalidArgumentException',     'include' => 'ext/spl/spl_exceptions.h'],
        'RangeException'           => ['ce' => 'spl_ce_RangeException',               'include' => 'ext/spl/spl_exceptions.h'],
    ];
    if (isset($extern[$fqn])) {
        return ['mode' => 'extern'] + $extern[$fqn];
    }
    if (str_starts_with($fqn, 'Cassandra\\') || $fqn === 'Cassandra') {
        return ['mode' => 'registry', 'fqn' => $fqn];
    }
    // Unknown — treat as registry and hope it's there. Will surface at MINIT
    // if it's actually missing.
    return ['mode' => 'registry', 'fqn' => $fqn];
}

/**
 * "Cassandra\Foo\BarBaz" → "foo_bar_baz"
 * "Cassandra"             → "core" (we use `core` as the snake for the umbrella)
 */
function fqn_to_snake(string $fqn): string
{
    if ($fqn === 'Cassandra') {
        return 'core';
    }
    $tail = preg_replace('/^Cassandra\\\\/', '', $fqn);
    $parts = explode('\\', $tail);
    $snakeParts = array_map('pascal_to_snake', $parts);
    return implode('_', $snakeParts);
}

function pascal_to_snake(string $s): string
{
    // Insert _ before each interior uppercase letter that follows a lowercase
    // or another uppercase letter followed by lowercase (so HTTPResponse →
    // http_response, not h_t_t_p_response).
    $s = preg_replace('/([a-z0-9])([A-Z])/', '$1_$2', $s);
    $s = preg_replace('/([A-Z]+)([A-Z][a-z])/', '$1_$2', $s);
    return strtolower($s);
}

function fqn_to_register_fn(string $fqn): string
{
    // Cassandra\RetryPolicy\Fallthrough → register_class_Cassandra_RetryPolicy_Fallthrough
    return 'register_class_' . str_replace('\\', '_', $fqn);
}

function emit_descriptor_file(array $classes, string $arginfoHeader, string $stubBasename): string
{
    $extraIncludes = [];
    $bodies = [];
    $skipped = [];

    foreach ($classes as $cls) {
        if ($cls['no_generate']) {
            $skipped[] = $cls['fqn'];
            continue;
        }
        [$body, $includes] = emit_class_descriptor($cls);
        $bodies[] = $body;
        foreach ($includes as $inc) {
            $extraIncludes[$inc] = true;
        }
    }

    if (!$bodies) {
        $skippedList = $skipped ? implode(', ', $skipped) : '(none)';
        return <<<EOF
/* generated from $stubBasename — no descriptors emitted (skipped: $skippedList) */
EOF;
    }

    $extraIncBlock = '';
    foreach (array_keys($extraIncludes) as $inc) {
        $extraIncBlock .= "#include <$inc>\n";
    }

    return <<<EOF
/*
 * Generated from $stubBasename. DO NOT EDIT.
 *
 * Regenerated at build time by tools/gen_descriptor/gen_class_descriptor.php
 * via cmake/GenStubs.cmake's php_scylladb_generate_arginfo() function.
 */

#include <php.h>
#include <Zend/zend_attributes.h>
#include "$arginfoHeader"
#include <Registry/Registry.h>
$extraIncBlock

EOF
    . implode("\n", $bodies);
}

function emit_class_descriptor(array $cls): array
{
    $fqn   = $cls['fqn'];
    $snake = fqn_to_snake($fqn);
    $registerClassFn = fqn_to_register_fn($fqn);

    $registryDeps = [];
    $registerArgs = [];
    $extraIncludes = [];

    foreach ($cls['parents'] as $p) {
        $info = classify_parent($p['fqn']);
        if ($info['mode'] === 'registry') {
            $registryDeps[] = $info['fqn'];
            $registerArgs[] = 'deps[' . (count($registryDeps) - 1) . ']';
        } else {
            $registerArgs[] = $info['ce'];
            if ($info['include']) {
                $extraIncludes[] = $info['include'];
            }
        }
    }

    // FQN as a C string literal — backslashes need doubling.
    $fqnLit = str_replace('\\', '\\\\', $fqn);

    $ceVar       = "php_scylladb_{$snake}_ce";
    $handlersVar = "php_scylladb_{$snake}_handlers";
    $registerFn  = "php_scylladb_register_{$snake}";

    // Value-typed classes use a richer handlers struct that wraps the
    // standard zend_object_handlers with an extra `hash_value` callback.
    // Field access for standard handlers goes via `.std` when this is on.
    $useValueHandlers = !empty($cls['value_handlers']);
    $handlersType     = $useValueHandlers ? 'php_scylladb_value_handlers' : 'zend_object_handlers';
    $stdAccess        = $useValueHandlers ? '.std' : '';
    if ($useValueHandlers) {
        $extraIncludes[] = 'php_scylladb_types.h';
    }

    // Weak forward declarations for user-provided callbacks. If the user
    // didn't define them, the linker leaves the reference as NULL.
    $hooks = [
        'new'        => ['zend_object *', 'zend_class_entry *'],
        'free'       => ['void',          'zend_object *'],
        'properties' => ['HashTable *',   'zend_object *'],
        'gc'         => ['HashTable *',   'zend_object *, zval **, int *'],
        'compare'    => ['int',           'zval *, zval *'],
        'cast'       => ['zend_result',   'zend_object *, zval *, int'],
        'clone'      => ['zend_object *', 'zend_object *'],
    ];
    if ($useValueHandlers) {
        // Wired to the .hash_value slot on php_scylladb_value_handlers, not on
        // zend_object_handlers — so it's only available for value-flavored
        // classes.
        $hooks['hash_value'] = ['unsigned', 'zval *'];
    }

    $weakDecls = '';
    foreach ($hooks as $role => [$ret, $args]) {
        $name = "php_scylladb_{$snake}_{$role}";
        $weakDecls .= "extern __attribute__((weak)) $ret $name($args);\n";
    }
    $weakDecls .= "extern __attribute__((weak)) void php_scylladb_{$snake}_post_register(zend_class_entry *);\n";

    // The register fn:
    //   1. call register_class_X(...) with deps[i] / extern ce's
    //   2. set create_object if php_scylladb_<snake>_new is defined
    //   3. memcpy std handlers + apply each weak callback if defined
    //   4. call post_register if defined
    //   5. return ce

    $callArgs = implode(', ', $registerArgs);
    $registerCall = $callArgs === ''
        ? "  zend_class_entry *ce = $registerClassFn();\n"
        : "  zend_class_entry *ce = $registerClassFn($callArgs);\n";

    // Handler wiring. The memcpy initializes the std-handlers slot (which is
    // `<handlersVar>$stdAccess`); any user-overridden callbacks get wired in
    // after. The hash_value extension only exists on value_handlers.
    $handlerWiring = "  memcpy(&$handlersVar$stdAccess, zend_get_std_object_handlers(), sizeof(zend_object_handlers));\n";
    $handlerWiring .= "  if (&php_scylladb_{$snake}_free)       $handlersVar$stdAccess.free_obj       = php_scylladb_{$snake}_free;\n";
    $handlerWiring .= "  if (&php_scylladb_{$snake}_properties) $handlersVar$stdAccess.get_properties = php_scylladb_{$snake}_properties;\n";
    $handlerWiring .= "  if (&php_scylladb_{$snake}_gc)         $handlersVar$stdAccess.get_gc         = php_scylladb_{$snake}_gc;\n";
    $handlerWiring .= "  if (&php_scylladb_{$snake}_compare)    $handlersVar$stdAccess.compare        = php_scylladb_{$snake}_compare;\n";
    $handlerWiring .= "  if (&php_scylladb_{$snake}_cast)       $handlersVar$stdAccess.cast_object    = php_scylladb_{$snake}_cast;\n";
    $handlerWiring .= "  if (&php_scylladb_{$snake}_clone)      $handlersVar$stdAccess.clone_obj      = php_scylladb_{$snake}_clone;\n";
    if ($useValueHandlers) {
        $handlerWiring .= "  if (&php_scylladb_{$snake}_hash_value) $handlersVar.hash_value           = php_scylladb_{$snake}_hash_value;\n";
    }

    $createObjWire = "  if (&php_scylladb_{$snake}_new) ce->create_object = php_scylladb_{$snake}_new;\n";
    $postReg = "  if (&php_scylladb_{$snake}_post_register) php_scylladb_{$snake}_post_register(ce);\n";

    // Descriptor macro selection: single parent shortcut vs deps array.
    if (count($registryDeps) === 0) {
        $macro = sprintf(
            "PHP_SCYLLADB_REGISTER_CLASS(\n    %s,\n    \"%s\",\n    &%s,\n    nullptr,\n    %s\n)\n",
            $snake, $fqnLit, $ceVar, $registerFn
        );
    } elseif (count($registryDeps) === 1) {
        $parentLit = str_replace('\\', '\\\\', $registryDeps[0]);
        $macro = sprintf(
            "PHP_SCYLLADB_REGISTER_CLASS(\n    %s,\n    \"%s\",\n    &%s,\n    \"%s\",\n    %s\n)\n",
            $snake, $fqnLit, $ceVar, $parentLit, $registerFn
        );
    } else {
        $depsArrName = "scylladb_{$snake}_registry_deps";
        $depsArr = "static const char *const $depsArrName" . "[] = {\n";
        foreach ($registryDeps as $d) {
            $dLit = str_replace('\\', '\\\\', $d);
            $depsArr .= "    \"$dLit\",\n";
        }
        $depsArr .= "    nullptr,\n};\n";

        $macro = $depsArr;
        $macro .= sprintf(
            "PHP_SCYLLADB_REGISTER_CLASS_DEPS(\n    %s,\n    \"%s\",\n    &%s,\n    %s,\n    %s\n)\n",
            $snake, $fqnLit, $ceVar, $depsArrName, $registerFn
        );
    }

    $body = <<<EOF

/* ─── $fqn ─── */

zend_class_entry      *$ceVar       = nullptr;
$handlersType   $handlersVar;

#ifdef __cplusplus
extern "C" {
#endif

$weakDecls
static zend_class_entry *$registerFn(zend_class_entry *const *deps)
{
  (void)deps;
$registerCall
$createObjWire
$handlerWiring
$postReg
  return ce;
}

#ifdef __cplusplus
}
#endif

$macro
EOF;

    return [$body, $extraIncludes];
}
