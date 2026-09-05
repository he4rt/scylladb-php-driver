<?php

/**
 * Generate IDE stubs from the src/**\/*.stub.php sources.
 *
 * The .stub.php files are the source of truth for the extension's PHP-visible
 * API, but they are not usable as IDE stubs directly:
 *
 *   - `@cvalue` constants are declared as `= UNKNOWN`; the real values live in
 *     cassandra.h and are only known to a loaded extension.
 *   - `@generate-class-entries`, `@strict-properties`, `@scylladb-struct` and
 *     friends are build-time directives that mean nothing to an IDE.
 *
 * This tool resolves the constants against the loaded extension and strips the
 * build-time annotations, so PhpStorm and VS Code see the same signatures the
 * engine registers.
 *
 * Usage: php -d extension=cassandra tools/gen_ide_stubs.php [output-dir]
 */

declare(strict_types=1);

namespace ScyllaDB\Tools\IdeStubs;

use PhpParser\Node;
use PhpParser\NodeTraverser;
use PhpParser\NodeVisitorAbstract;
use PhpParser\ParserFactory;
use PhpParser\PrettyPrinter;

const EXTENSION = 'cassandra';

require_once autoloader(dirname(__DIR__));

/** Build-time annotations that carry no meaning for an IDE. */
const INTERNAL_ANNOTATIONS = [
    'generate-class-entries',
    'strict-properties',
    'not-serializable',
    'scylladb-struct',
    'scylladb-value-handlers',
    'cvalue',
    'undocumentable',
];

function main(array $argv): int
{
    $root = dirname(__DIR__);
    $outDir = $argv[1] ?? $root . '/build/ide-stubs';

    if (!extension_loaded(EXTENSION)) {
        fwrite(STDERR, "gen_ide_stubs: the '" . EXTENSION . "' extension must be loaded.\n");
        fwrite(STDERR, "Run: php -d extension=cassandra tools/gen_ide_stubs.php\n");

        return 1;
    }

    $version = (string) phpversion(EXTENSION);
    $parser = (new ParserFactory())->createForNewestSupportedVersion();
    $printer = new PrettyPrinter\Standard(['shortArraySyntax' => true]);

    $stubs = findStubs($root . '/src');
    if ($stubs === []) {
        fwrite(STDERR, "gen_ide_stubs: no .stub.php files found under src/.\n");

        return 1;
    }

    removeTree($outDir);
    @mkdir($outDir, 0o777, true);

    $written = 0;
    $unresolved = [];

    foreach ($stubs as $stub) {
        $ast = $parser->parse((string) file_get_contents($stub));
        if ($ast === null) {
            fwrite(STDERR, "gen_ide_stubs: failed to parse $stub\n");

            return 1;
        }

        $traverser = new NodeTraverser();
        $traverser->addVisitor($visitor = new StubVisitor());
        $ast = $traverser->traverse($ast);

        $unresolved = [...$unresolved, ...$visitor->unresolved];

        $relative = substr($stub, strlen($root . '/src/'));
        $target = $outDir . '/' . preg_replace('/\.stub\.php$/', '.php', $relative);

        @mkdir(dirname($target), 0o777, true);
        file_put_contents($target, header() . ltrim(substr($printer->prettyPrintFile($ast), 5)));
        $written++;
    }

    writePackaging($outDir, $version);

    printf("gen_ide_stubs: wrote %d files to %s\n", $written, $outDir);

    if ($unresolved !== []) {
        fwrite(STDERR, "gen_ide_stubs: could not resolve " . count($unresolved) . " constant(s):\n");
        foreach ($unresolved as $name) {
            fwrite(STDERR, "  - $name\n");
        }

        return 1;
    }

    return verify($outDir, $parser) ? 0 : 1;
}

// ─── nodes ───────────────────────────────────────────────────────────────────

final class StubVisitor extends NodeVisitorAbstract
{
    /** @var list<string> */
    public array $unresolved = [];

    private ?string $namespace = null;
    private ?string $class = null;

    public function enterNode(Node $node): null
    {
        if ($node instanceof Node\Stmt\Namespace_) {
            $this->namespace = $node->name?->toString();
        }
        if ($node instanceof Node\Stmt\ClassLike && $node->name !== null) {
            $this->class = $node->name->toString();
        }

        return null;
    }

    public function leaveNode(Node $node): null|int|Node
    {
        $this->stripAnnotations($node);

        // `declare(strict_types=1)` has no effect in a file that never runs.
        if ($node instanceof Node\Stmt\Declare_ && $node->stmts === null) {
            return NodeTraverser::REMOVE_NODE;
        }

        if ($node instanceof Node\Stmt\ClassConst) {
            $this->resolveConstants($node);
        }

        return null;
    }

    /** Replace `= UNKNOWN` placeholders with the value the extension registered. */
    private function resolveConstants(Node\Stmt\ClassConst $node): void
    {
        foreach ($node->consts as $const) {
            if (!$const->value instanceof Node\Expr\ConstFetch) {
                continue;
            }
            if ($const->value->name->toString() !== 'UNKNOWN') {
                continue;
            }

            $fqcn = $this->namespace === null || $this->namespace === ''
                ? (string) $this->class
                : $this->namespace . '\\' . $this->class;
            $name = $fqcn . '::' . $const->name->toString();

            if (!defined($name)) {
                $this->unresolved[] = $name;
                continue;
            }

            $const->value = literal(constant($name));
        }
    }

    private function stripAnnotations(Node $node): void
    {
        $doc = $node->getDocComment();
        if ($doc === null) {
            return;
        }

        $kept = [];
        foreach (explode("\n", $doc->getText()) as $line) {
            if (preg_match('/@(' . implode('|', INTERNAL_ANNOTATIONS) . ')\b/', $line)) {
                continue;
            }
            $kept[] = $line;
        }

        // Everything but the `/**`, the `*/` and blank `*` lines went away.
        $meaningful = array_filter(
            $kept,
            static fn(string $l): bool => preg_match('#^\s*(/\*\*|\*/|\*\s*)$#', $l) !== 1,
        );

        if ($meaningful === []) {
            $node->setAttribute('comments', []);

            return;
        }

        $node->setDocComment(new \PhpParser\Comment\Doc(implode("\n", $kept)));
    }
}

// ─── helpers ─────────────────────────────────────────────────────────────────

function literal(mixed $value): Node\Expr
{
    return match (true) {
        is_int($value) => $value < 0
            ? new Node\Expr\UnaryMinus(new Node\Scalar\Int_(-$value))
            : new Node\Scalar\Int_($value),
        is_float($value) => new Node\Scalar\Float_($value),
        is_bool($value) => new Node\Expr\ConstFetch(new Node\Name($value ? 'true' : 'false')),
        $value === null => new Node\Expr\ConstFetch(new Node\Name('null')),
        default => new Node\Scalar\String_((string) $value),
    };
}

function header(): string
{
    return "<?php\n\n"
        . "/**\n"
        . " * IDE stubs for the ScyllaDB PHP driver (ext-" . EXTENSION . ").\n"
        . " *\n"
        . " * Generated from the extension's .stub.php sources by tools/gen_ide_stubs.php.\n"
        . " * Do not edit. These declarations exist for editor completion only; the real\n"
        . " * implementations live in the compiled extension.\n"
        . " */\n\n";
}

/** @return list<string> */
function findStubs(string $dir): array
{
    $found = [];
    $it = new \RecursiveIteratorIterator(new \RecursiveDirectoryIterator($dir));
    foreach ($it as $file) {
        if ($file->isFile() && str_ends_with($file->getFilename(), '.stub.php')) {
            $found[] = $file->getPathname();
        }
    }
    sort($found);

    return $found;
}

function autoloader(string $root): string
{
    $vendored = glob($root . '/tools/gen_stub/PHP-Parser-*/lib/PhpParser', GLOB_ONLYDIR) ?: [];
    usort($vendored, static fn (string $a, string $b): int => version_compare(
        basename(dirname($a, 2)),
        basename(dirname($b, 2)),
    ));
    if ($vendored !== []) {
        $lib = dirname(end($vendored));
        spl_autoload_register(static function (string $class) use ($lib): void {
            if (!str_starts_with($class, 'PhpParser\\')) {
                return;
            }
            $path = $lib . '/' . str_replace('\\', '/', $class) . '.php';
            if (file_exists($path)) {
                require_once $path;
            }
        });

        return $root . '/tools/gen_ide_stubs.php';
    }

    return $root . '/vendor/autoload.php';
}

function removeTree(string $dir): void
{
    if (!is_dir($dir)) {
        return;
    }
    $it = new \RecursiveIteratorIterator(
        new \RecursiveDirectoryIterator($dir, \FilesystemIterator::SKIP_DOTS),
        \RecursiveIteratorIterator::CHILD_FIRST,
    );
    foreach ($it as $file) {
        $file->isDir() ? @rmdir($file->getPathname()) : @unlink($file->getPathname());
    }
    @rmdir($dir);
}

function writePackaging(string $outDir, string $version): void
{
    $composer = [
        'name' => 'scylladb/php-driver-stubs',
        'description' => 'IDE stubs for the ScyllaDB PHP driver (ext-cassandra).',
        'license' => 'Apache-2.0',
        'type' => 'library',
        'keywords' => ['scylladb', 'cassandra', 'stubs', 'ide', 'phpstorm', 'static analysis'],
        'require' => ['php' => '>=8.3'],
    ];

    file_put_contents(
        $outDir . '/composer.json',
        json_encode($composer, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES) . "\n",
    );

    $readme = <<<MD
        # ScyllaDB PHP driver — IDE stubs

        Editor completion and static-analysis data for `ext-cassandra` version {$version}.
        The files declare the extension's classes, methods and constants. They contain
        no implementation — load the real extension to run code.

        ## Install with Composer

            composer require --dev scylladb/php-driver-stubs

        The package declares no autoloader, so it cannot shadow the real extension at
        runtime. PhpStorm and VS Code index `vendor/` and pick the stubs up on their own.

        ## Install by hand

        Unpack the archive somewhere in your project and point your editor at it.

        - **PhpStorm** — *Settings → PHP → Include Path*, then add the directory.
        - **VS Code (Intelephense)** — add the directory to `intelephense.environment.includePaths`.

        ## Static analysis

        PHPStan and Psalm read the stubs through Composer. To point them at an unpacked
        copy instead, add the directory to `scanDirectories` (PHPStan) or `<stubs>`
        (Psalm).

        Generated from the extension's `.stub.php` sources. Do not edit by hand.
        MD;

    file_put_contents($outDir . '/README.md', $readme . "\n");
}

/**
 * Compare what the loaded extension registers against what the stubs declare.
 * A gap means the stubs drifted from the C sources.
 */
function verify(string $outDir, \PhpParser\Parser $parser): bool
{
    $declared = [];
    $it = new \RecursiveIteratorIterator(new \RecursiveDirectoryIterator($outDir));
    foreach ($it as $file) {
        if (!$file->isFile() || $file->getExtension() !== 'php') {
            continue;
        }
        $ast = $parser->parse((string) file_get_contents($file->getPathname())) ?? [];
        $traverser = new NodeTraverser();
        $traverser->addVisitor($collector = new class extends NodeVisitorAbstract {
            /** @var list<string> */
            public array $names = [];
            private string $ns = '';

            public function enterNode(Node $node): null
            {
                if ($node instanceof Node\Stmt\Namespace_) {
                    $this->ns = $node->name?->toString() ?? '';
                }
                if ($node instanceof Node\Stmt\ClassLike && $node->name !== null) {
                    $this->names[] = ltrim($this->ns . '\\' . $node->name->toString(), '\\');
                }

                return null;
            }
        });
        $traverser->traverse($ast);
        $declared = [...$declared, ...$collector->names];
    }

    $declaredLower = array_map(strtolower(...), $declared);
    $actual = (new \ReflectionExtension(EXTENSION))->getClassNames();

    $missing = array_values(array_filter(
        $actual,
        static fn(string $c): bool => !in_array(strtolower($c), $declaredLower, true),
    ));

    if ($missing !== []) {
        fwrite(STDERR, "gen_ide_stubs: " . count($missing) . " class(es) exposed by the extension but absent from the stubs:\n");
        foreach ($missing as $class) {
            fwrite(STDERR, "  - $class\n");
        }

        return false;
    }

    printf("gen_ide_stubs: verified %d classes against the loaded extension\n", count($actual));

    return true;
}

exit(main($argv));
