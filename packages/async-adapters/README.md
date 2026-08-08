# ScyllaDB PHP driver — event-loop adapters

Optional glue between `Cassandra\Future` and third-party event loops. The
extension itself is pure C and needs none of this.

```bash
composer require codelieutenant/scylla-driver-async-adapters
```

| Class | Loop | Needs |
| --- | --- | --- |
| `Cassandra\Async\Revolt` | Revolt (fiber await) | `revolt/event-loop` |
| `Cassandra\Async\ReactorRevolt` | Revolt over the shared reactor | `revolt/event-loop` |
| `Cassandra\Async\Amp` | Amp v3 | `amphp/amp` |
| `Cassandra\Async\ReactPhp` | ReactPHP promises | `react/event-loop`, `react/promise` |
| `Cassandra\Async\Swoole` | Swoole coroutines | `ext-swoole` |

Each one is a thin wrapper over `Future::getResource()` or
`Cassandra\Async\Reactor`. They live outside the extension because they call
framework classes that the C code cannot see, and because a framework API change
must not need an extension rebuild.

## You may not need them

Two integration points are native to the extension:

- **PHP 8.6 or later** — `Cassandra\Async\Poll` is a complete event loop over
  `Io\Poll\Context`, written in C. No package, no framework.
- **Swoole** — build the extension with `--enable-swoole` and `Future::get()`
  suspends the calling coroutine on its own. `Cassandra\Async\Swoole` is only
  the fallback for builds without that flag.

See `docs/async.md` in the driver repository.
