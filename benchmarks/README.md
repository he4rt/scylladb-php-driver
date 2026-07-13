# Benchmarks

Performance suite for the `cassandra` extension, driven by
[PhpBench](https://phpbench.readthedocs.io/). It is split into two groups:

| Group     | Needs a DB? | What it measures |
|-----------|-------------|------------------|
| `offline` | no          | Pure-CPU cost of the extension: value marshalling, statement/collection construction, and the cluster-builder config path. Deterministic and CI-friendly. |
| `live`    | yes         | Real round-trip behaviour against a running ScyllaDB/Cassandra: connect, execute (simple vs prepared), writes/batches, result paging, and async pipelining. |

Every subject reports **wall-clock time** (`mode`, with `rstdev` as the spread)
and **peak memory** (`mem_peak`). For the extension, memory is as important as
time — an extra copy or a leak on a hot path shows up in `mem_peak` first.

## Layout

```
benchmarks/
├── bootstrap.php              # autoload + "is the extension loaded?" guard
├── run.sh                     # wrapper that points PhpBench at a local build
├── Support/
│   ├── Env.php                # SCYLLADB_* connection config (shared with tests)
│   ├── Sizes.php              # shared param providers (element counts, byte sizes)
│   └── LiveBenchCase.php      # base class: builder, connect, schema setup
├── Offline/
│   ├── ValueBench.php         # Uuid/Bigint/Decimal/Varint/Timestamp/Inet/Blob …
│   ├── CollectionBench.php    # Set/Map/List/Tuple/UDT build + read, scaled by size
│   ├── BlobBench.php          # blob construct/readback across payload sizes
│   ├── StatementBench.php     # SimpleStatement / BatchStatement construction
│   └── ClusterBuilderBench.php# builder chain + build(); persistent ON vs OFF
└── Live/
    ├── ConnectBench.php       # build()+connect()+close(), persistent ON vs OFF
    ├── ExecuteBench.php       # simple vs prepared SELECT, bound lookup
    ├── WriteBench.php         # single insert vs UNLOGGED batch (scaled)
    ├── PagingBench.php        # full scan at different page sizes
    └── AsyncBench.php         # executeAsync pipelining vs sequential execute
```

## Running

The extension must be loaded in the PHP that PhpBench spawns. `benchmarks/run.sh`
handles that for a local dev build (module under `out/…`) by autodetecting the
`.so`/`.dylib` and passing the right flags; if the extension is installed
system-wide it just relies on `phpbench.json` (`extension=cassandra`).

```bash
composer install                       # first time — pulls in phpbench

benchmarks/run.sh --group=offline      # offline micro-benchmarks (no DB)
benchmarks/run.sh --group=live         # live benchmarks (needs a node)
benchmarks/run.sh                      # everything
benchmarks/run.sh benchmarks/Live/AsyncBench.php   # a single file
benchmarks/run.sh --filter=benchPrepared           # a single subject
```

Point it at a specific build or PHP with env vars:

```bash
EXT=out/RelWithDebInfoPHP8.4NTS/cassandra.dylib \
PHP=php/8.4-debug-nts/bin/php \
benchmarks/run.sh --group=offline
```

The `composer bench*` scripts do the same via `phpbench` directly and assume the
extension is installed by name (best for CI):

```bash
composer bench            # aggregate report, all groups
composer bench:offline
composer bench:live
```

### Live benchmarks — bring up a node

```bash
./scripts/run-scylladb.sh          # docker compose up (localhost:9042)
benchmarks/run.sh --group=live
```

Connection settings come from the same environment variables as the test suite,
all optional:

| Variable | Default |
|----------|---------|
| `SCYLLADB_HOSTS` | `127.0.0.1` |
| `SCYLLADB_PORT` | `9042` |
| `SCYLLADB_USERNAME` | `cassandra` |
| `SCYLLADB_PASSWORD` | `cassandra` |
| `SCYLLADB_BENCH_KEYSPACE` | `bench_scylladb` |

The live group creates the `bench_scylladb` keyspace (`IF NOT EXISTS`) and reuses
it. It is **not** dropped automatically — dropping it would race across
PhpBench's per-iteration subprocesses. Remove it when you are done:

```bash
benchmarks/run.sh --group=live      # ... then:
php -r '/* your one-off */' # or from cqlsh:
#   DROP KEYSPACE IF EXISTS bench_scylladb;
```

## Tracking regressions with baselines

PhpBench can store a run and diff against it — this is how you catch a
performance regression in a PR.

```bash
# 1. On the base commit, snapshot a baseline:
benchmarks/run.sh --group=offline --store --tag=baseline

# 2. On your branch, compare against it:
benchmarks/run.sh --group=offline --ref=baseline --report=aggregate
```

With `--ref`, the `aggregate` report annotates every `mode`/`mem_peak`/`rstdev`
cell with the percentage delta versus the baseline, so a regression is a
red `+NN%` in the time column. Baselines live under `.phpbench/` (git-ignored).
Keep the offline
group as the regression gate: it is deterministic, whereas live timings depend on
the node, network, and load.

## How it is measured

- PhpBench runs each **iteration** in its own subprocess and loops the
  configured **revolutions** inside it; the reported time is per revolution.
- Offline subjects use high revolution counts (fast, pure-CPU) for tight
  confidence intervals. Parameterised subjects (`set` column) sweep a size axis
  so non-linear cost is obvious.
- Live subjects open one session per iteration in a `@BeforeMethods` hook, so the
  connection cost is paid once and amortised; only the operation under test is
  timed. Revolution counts are low because a round trip dwarfs any local work.
- Watch `rstdev`: under a few percent is trustworthy. A noisy live number usually
  means a busy or contended node, not a code change.
