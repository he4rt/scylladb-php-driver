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
 *   php gen_class_descriptor.php <stub.php> <out.c> <arginfo_header>
 */

declare(strict_types=1);

if ($argc !== 4) {
    fwrite(STDERR, "usage: gen_class_descriptor.php <stub.php> <out.c> <arginfo_header>\n");
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
 *     'kind'       => 'class' | 'interface' | 'enum',
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

        if ($id === T_CLASS || $id === T_INTERFACE || $id === T_ENUM) {
            $kind = match ($id) {
                T_INTERFACE => 'interface',
                T_ENUM      => 'enum',
                default     => 'class',
            };

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
            $structName = null;
            if ($doc !== null && preg_match('/@scylladb-struct\s+([A-Za-z_][A-Za-z0-9_]*)/', $doc, $m)) {
                $structName = $m[1];
            }
            $out[] = [
                'fqn'              => $fqn,
                'kind'             => $kind,
                'is_final'         => $isFinal,
                'is_abstract'      => $isAbstract,
                'parents'          => $parents,
                'no_generate'      => $doc !== null && (str_contains($doc, '@scylladb-no-generate')
                                                       || str_contains($doc, '@scylladb-skip')),
                'value_handlers'   => $hasValueHandlers,
                'struct'           => $structName,
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

/* Include the umbrella header so any @cvalue identifier (CASS_*,
 * PHP_SCYLLADB_VERSION, …) is visible to the arginfo register fn. */
#include "php_scylladb.h"
#include <Zend/zend_attributes.h>
$extraIncBlock#include "$arginfoHeader"
#include <Registry/Registry.h>

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

    // Enums own no instances the extension allocates: cases are created by
    // zend_register_internal_enum() and are immutable. No create_object, no
    // handlers table, no offset — only the post_register hook stays.
    if ($cls['kind'] === 'enum') {
        return emit_enum_descriptor($cls, $snake, $ceVar, $registerFn, $fqnLit, $registerClassFn, $registryDeps, $registerArgs, $extraIncludes);
    }

    // Value-typed classes use a richer handlers struct that wraps the
    // standard zend_object_handlers with an extra `hash_value` callback.
    // Field access for standard handlers goes via `.std` when this is on.
    $useValueHandlers = !empty($cls['value_handlers']);
    $handlersType     = $useValueHandlers ? 'php_scylladb_value_handlers' : 'zend_object_handlers';
    $stdAccess        = $useValueHandlers ? '.std' : '';
    // Both the value-handlers type and any @scylladb-struct typename live in
    // php_scylladb_types.h (or modules that include it transitively).
    if ($useValueHandlers || !empty($cls['struct'])) {
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
    //
    // If a @scylladb-struct annotation specified the embedding type,
    // set handlers.offset so Zend can locate the user struct's start from
    // the embedded zend_object pointer (free / property access / etc.).
    // Without this, offset stays 0 and Zend efrees/derefs the wrong
    // address → zend_mm_heap corrupted at runtime.
    $handlerWiring = "  memcpy(&$handlersVar$stdAccess, zend_get_std_object_handlers(), sizeof(zend_object_handlers));\n";
    if (!empty($cls['struct']) && $cls['struct'] !== 'none') {
        $struct = $cls['struct'];
        $handlerWiring .= "  $handlersVar$stdAccess.offset = offsetof($struct, zendObject);\n";
    } elseif (empty($cls['struct'])) {
        // No annotation at all. If the module nonetheless supplies a custom
        // create_object, offset stays 0 and Zend efrees the zend_object
        // instead of the enclosing struct — a leak at best, a corrupted heap
        // at worst. The weak symbol is only resolved at link time, so this
        // has to be a load-time check rather than a static assertion.
        // `@scylladb-struct none` opts out for classes that really do
        // allocate a bare zend_object.
        $handlerWiring .= "  if (&php_scylladb_{$snake}_new) {\n"
            . "    zend_error_noreturn(E_CORE_ERROR,\n"
            . "        \"$fqnLit defines php_scylladb_{$snake}_new() but its stub has no \"\n"
            . "        \"@scylladb-struct annotation, so handlers.offset stays 0. Add \"\n"
            . "        \"@scylladb-struct <type> to the stub, or @scylladb-struct none \"\n"
            . "        \"if the class allocates a bare zend_object.\");\n"
            . "  }\n";
    }
    $handlerWiring .= "  if (&php_scylladb_{$snake}_free)       $handlersVar$stdAccess.free_obj       = php_scylladb_{$snake}_free;\n";
    $handlerWiring .= "  if (&php_scylladb_{$snake}_properties) $handlersVar$stdAccess.get_properties = php_scylladb_{$snake}_properties;\n";
    $handlerWiring .= "  if (&php_scylladb_{$snake}_gc)         $handlersVar$stdAccess.get_gc         = php_scylladb_{$snake}_gc;\n";
    $handlerWiring .= "  if (&php_scylladb_{$snake}_compare)    $handlersVar$stdAccess.compare        = php_scylladb_{$snake}_compare;\n";
    $handlerWiring .= "  if (&php_scylladb_{$snake}_cast)       $handlersVar$stdAccess.cast_object    = php_scylladb_{$snake}_cast;\n";
    // Cloning: assign unconditionally from the weakly-declared _clone fn. If
    // the user defined it, that's their custom clone handler; if they didn't,
    // the weak ref resolves to NULL → clone_obj = nullptr disables cloning
    // entirely (PHP throws on `clone $obj`). This matches the pre-refactor
    // behavior where every class explicitly set clone_obj = NULL because the
    // underlying C state (CassFuture*, CassStatement*, etc.) can't safely be
    // shallow-copied.
    $handlerWiring .= "  $handlersVar$stdAccess.clone_obj = php_scylladb_{$snake}_clone;\n";
    if ($useValueHandlers) {
        $handlerWiring .= "  if (&php_scylladb_{$snake}_hash_value) $handlersVar.hash_value           = php_scylladb_{$snake}_hash_value;\n";
    }

    $createObjWire = "  if (&php_scylladb_{$snake}_new) ce->create_object = php_scylladb_{$snake}_new;\n";
    $postReg = "  if (&php_scylladb_{$snake}_post_register) php_scylladb_{$snake}_post_register(ce);\n";

    $macro = emit_registry_macro($snake, $fqnLit, $ceVar, $registerFn, $registryDeps);

    $body = <<<EOF

/* ─── $fqn ─── */

zend_class_entry      *$ceVar       = nullptr;
$handlersType   $handlersVar;

$weakDecls
static zend_class_entry *$registerFn([[maybe_unused]] zend_class_entry *const *deps)
{
$registerCall
$createObjWire
$handlerWiring
$postReg
  return ce;
}

$macro
EOF;

    return [$body, $extraIncludes];
}

/**
 * Descriptor macro selection: no parent, single parent shortcut, or deps array.
 */
function emit_registry_macro(string $snake, string $fqnLit, string $ceVar, string $registerFn, array $registryDeps): string
{
    if (count($registryDeps) === 0) {
        return sprintf(
            "PHP_SCYLLADB_REGISTER_CLASS(\n    %s,\n    \"%s\",\n    &%s,\n    nullptr,\n    %s\n)\n",
            $snake, $fqnLit, $ceVar, $registerFn
        );
    }

    if (count($registryDeps) === 1) {
        $parentLit = str_replace('\\', '\\\\', $registryDeps[0]);
        return sprintf(
            "PHP_SCYLLADB_REGISTER_CLASS(\n    %s,\n    \"%s\",\n    &%s,\n    \"%s\",\n    %s\n)\n",
            $snake, $fqnLit, $ceVar, $parentLit, $registerFn
        );
    }

    $depsArrName = "scylladb_{$snake}_registry_deps";
    $macro = "static const char *const $depsArrName" . "[] = {\n";
    foreach ($registryDeps as $d) {
        $dLit = str_replace('\\', '\\\\', $d);
        $macro .= "    \"$dLit\",\n";
    }
    $macro .= "    nullptr,\n};\n";

    return $macro . sprintf(
        "PHP_SCYLLADB_REGISTER_CLASS_DEPS(\n    %s,\n    \"%s\",\n    &%s,\n    %s,\n    %s\n)\n",
        $snake, $fqnLit, $ceVar, $depsArrName, $registerFn
    );
}

/**
 * An internal enum registers through the same register_class_*() entry point
 * that gen_stub emits for classes, but it has no object layout of its own:
 * cases are immutable objects the engine builds. So the descriptor drops
 * create_object, the handlers table and every handler hook.
 */
function emit_enum_descriptor(
    array $cls,
    string $snake,
    string $ceVar,
    string $registerFn,
    string $fqnLit,
    string $registerClassFn,
    array $registryDeps,
    array $registerArgs,
    array $extraIncludes
): array {
    $fqn = $cls['fqn'];

    // zend_register_internal_enum() / zend_enum_add_case_cstr() live here, and
    // the arginfo header calls both without including anything itself.
    $extraIncludes[] = 'Zend/zend_enum.h';

    $callArgs = implode(', ', $registerArgs);
    $registerCall = $callArgs === ''
        ? "  zend_class_entry *ce = $registerClassFn();\n"
        : "  zend_class_entry *ce = $registerClassFn($callArgs);\n";

    $macro = emit_registry_macro($snake, $fqnLit, $ceVar, $registerFn, $registryDeps);

    $body = <<<EOF

/* ─── $fqn (enum) ─── */

zend_class_entry      *$ceVar       = nullptr;

extern __attribute__((weak)) void php_scylladb_{$snake}_post_register(zend_class_entry *);

static zend_class_entry *$registerFn([[maybe_unused]] zend_class_entry *const *deps)
{
$registerCall
  if (&php_scylladb_{$snake}_post_register) php_scylladb_{$snake}_post_register(ce);

  return ce;
}

$macro
EOF;

    return [$body, $extraIncludes];
}
