<?php

declare(strict_types=1);

use Cassandra\Async\Poll;
use Cassandra\Async\PollHandle;
use Cassandra\Async\Reactor;
use Cassandra\FutureRows;
use Cassandra\SimpleStatement;

const POLL_CQL = 'SELECT release_version FROM system.local';

/**
 * Io\Poll integration — only built against PHP >= 8.6.
 */
beforeEach(function () {
    if (!class_exists(PollHandle::class)) {
        test()->markTestSkipped('Built without Io\\Poll support (needs PHP >= 8.6)');
    }
});

/**
 * Context::wait() took (?int $seconds, int $micros, ?int $max) up to 8.6.0alpha3
 * and takes (?Time\Duration, ?int $max) after it. Pick the one this build has.
 */
function pollWait(Io\Poll\Context $ctx, int $seconds): array
{
    $type = (string) (new ReflectionMethod($ctx, 'wait'))->getParameters()[0]->getType();

    return str_contains($type, 'Duration')
        ? $ctx->wait(Time\Duration::fromSeconds($seconds))
        : $ctx->wait($seconds);
}

it('exposes the reactor as a native poll handle', function () {
    $handle = PollHandle::reactor();

    expect($handle)->toBeInstanceOf(Io\Poll\Handle::class)
        ->and($handle->isValid())->toBeTrue()
        ->and($handle->getFileDescriptor())->toBeGreaterThanOrEqual(0)
        // Cached per request — one watcher per context.
        ->and(PollHandle::reactor())->toBe($handle);
})->group('feature', 'async', 'poll');

it('dispatches many concurrent futures through one poll context', function () {
    $session = scyllaDbConnection();
    $count   = 256;

    for ($i = 0; $i < $count; $i++) {
        Reactor::add($session->executeAsync(new SimpleStatement(POLL_CQL)));
    }
    expect(Reactor::pending())->toBe($count);

    $ctx = new Io\Poll\Context();
    $ctx->add(PollHandle::reactor(), [Io\Poll\Event::Read]);

    $done = 0;
    while (Reactor::pending() > 0) {
        expect(pollWait($ctx, 5))->not->toBeEmpty();

        foreach (Reactor::poll(64) as $future) {
            expect($future)->toBeInstanceOf(FutureRows::class)
                ->and($future->get()->count())->toBeGreaterThan(0);
            $done++;
        }
    }

    expect($done)->toBe($count)
        ->and(Reactor::pending())->toBe(0);
})->group('feature', 'async', 'poll');

it('watches a single future without a stream resource', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(POLL_CQL));

    $ctx = new Io\Poll\Context();
    $ctx->add(PollHandle::forFuture($future), [Io\Poll\Event::Read]);

    expect(pollWait($ctx, 5))->not->toBeEmpty()
        ->and($future->get()->count())->toBeGreaterThan(0);
})->group('feature', 'async', 'poll');

it('drives many futures through the Poll loop with callbacks', function () {
    $session = scyllaDbConnection();
    $count   = 64;

    $loop = new Cassandra\Async\Poll();
    $rows = 0;

    for ($i = 0; $i < $count; $i++) {
        $loop->watch(
            $session->executeAsync(new SimpleStatement(POLL_CQL)),
            function (Cassandra\Future $future) use (&$rows) {
                $rows += $future->get()->count();
            },
        );
    }

    expect($loop->pending())->toBe($count);
    $loop->run(5);

    // run() must drop every watcher, or the loop would never return.
    expect($rows)->toBe($count)
        ->and($loop->pending())->toBe(0);
})->group('feature', 'async', 'poll');

it('returns futures from tick() when no callback is given', function () {
    $session = scyllaDbConnection();

    $loop = new Cassandra\Async\Poll();
    $loop->watch($session->executeAsync(new SimpleStatement(POLL_CQL)));

    $resolved = $loop->tick(5);

    expect($resolved)->toHaveCount(1)
        ->and($resolved[0])->toBeInstanceOf(FutureRows::class)
        ->and($resolved[0]->get()->count())->toBeGreaterThan(0)
        ->and($loop->pending())->toBe(0);
})->group('feature', 'async', 'poll');

it('refuses a future that is already registered with the reactor', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(POLL_CQL));
    Reactor::add($future);

    expect(fn () => PollHandle::forFuture($future))
        ->toThrow(Cassandra\Exception\RuntimeException::class);

    while (Reactor::pending() > 0) {
        Reactor::poll();
    }
})->group('feature', 'async', 'poll');

it('keeps callback-less futures watched when a callback throws', function () {
    $session = scyllaDbConnection();
    $loop    = new Poll();

    $plain = [];
    for ($i = 0; $i < 4; $i++) {
        $plain[] = $future = $session->executeAsync(new SimpleStatement(POLL_CQL));
        $loop->watch($future);
    }
    $thrower = $session->executeAsync(new SimpleStatement(POLL_CQL));
    $loop->watch($thrower, fn () => throw new LogicException('boom'));

    // Let every descriptor become readable, so one tick() sees all five at once
    // — that is the case where a one-pass dispatch would drop the four.
    $deadline = microtime(true) + 10.0;
    while (microtime(true) < $deadline) {
        $ready = $thrower->isReady();
        foreach ($plain as $future) {
            $ready = $ready && $future->isReady();
        }
        if ($ready) {
            break;
        }
        usleep(1000);
    }

    expect($loop->pending())->toBe(5);
    expect(fn () => $loop->tick(5))->toThrow(LogicException::class);

    // The throwing callback consumed only its own future; the four without a
    // callback are still watched, so nothing is lost.
    expect($loop->pending())->toBe(4);

    $returned = [];
    while ($loop->pending() > 0) {
        foreach ($loop->tick(5) as $future) {
            $returned[] = $future;
            $future->get();
        }
    }

    expect($returned)->toHaveCount(4);
})->group('feature', 'async', 'poll');

it('reports a live descriptor through the handle accessors', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(POLL_CQL));

    $handle = PollHandle::forFuture($future);

    expect($handle)->toBeInstanceOf(PollHandle::class)
        ->and($handle->isValid())->toBeTrue()
        ->and($handle->getFileDescriptor())->toBeGreaterThanOrEqual(0);

    // The reactor handle is cached per request: one object, one descriptor.
    expect(PollHandle::reactor())->toBe(PollHandle::reactor())
        ->and(PollHandle::reactor()->getFileDescriptor())->toBeGreaterThanOrEqual(0);

    // A per-future handle is NOT cached — each call is a fresh object over the
    // same notifier, so two handles report the same descriptor.
    expect(PollHandle::forFuture($future)->getFileDescriptor())
        ->toBe($handle->getFileDescriptor());

    $future->get();
})->group('feature', 'async', 'poll');

it('keeps the descriptor valid after the future is released', function () {
    $session = scyllaDbConnection();
    $future  = $session->executeAsync(new SimpleStatement(POLL_CQL));

    $handle = PollHandle::forFuture($future);
    $fd     = $handle->getFileDescriptor();

    unset($future);
    gc_collect_cycles();

    // The handle holds its own notifier reference, so the driver can still
    // write the wakeup without touching a freed descriptor.
    expect($handle->isValid())->toBeTrue()
        ->and($handle->getFileDescriptor())->toBe($fd);
})->group('feature', 'async', 'poll');

it('drives a loop to completion with run()', function () {
    $session = scyllaDbConnection();
    $loop    = new Poll();

    $done = 0;
    for ($i = 0; $i < 16; $i++) {
        $loop->watch(
            $session->executeAsync(new SimpleStatement(POLL_CQL)),
            function ($future) use (&$done) {
                expect($future->isReady())->toBeTrue();
                $future->get();
                $done++;
            },
        );
    }

    expect($loop->pending())->toBe(16);
    $loop->run(10);

    expect($done)->toBe(16)->and($loop->pending())->toBe(0);
})->group('feature', 'async', 'poll');

it('shares its context with watchers the caller adds', function () {
    $session = scyllaDbConnection();
    $loop    = new Poll();

    $watched = $session->executeAsync(new SimpleStatement(POLL_CQL));
    $loop->watch($watched);

    // The caller drives a second future on the same context by hand, without
    // going through watch() — proof that context() is the loop's own object and
    // not a copy.
    $manual  = $session->executeAsync(new SimpleStatement(POLL_CQL));
    $ctx     = $loop->context();
    $watcher = $ctx->add(PollHandle::forFuture($manual), [Io\Poll\Event::Read]);

    expect($ctx)->toBeInstanceOf(Io\Poll\Context::class)
        ->and($loop->context())->toBe($ctx)
        // The manual watcher is not the loop's business, so it is not counted.
        ->and($loop->pending())->toBe(1);

    $returned = [];
    while ($loop->pending() > 0) {
        foreach ($loop->tick(10) as $resolved) {
            $returned[] = $resolved;
            $resolved->get();
        }
    }

    // tick() dispatched only its own future and left the foreign watcher in
    // place for the caller to deal with.
    expect($returned)->toHaveCount(1)->and($returned[0])->toBe($watched);

    $watcher->remove();
    $manual->get();
})->group('feature', 'async', 'poll');

it('refuses to watch one future twice on the same context', function () {
    $session = scyllaDbConnection();
    $loop    = new Poll();
    $future  = $session->executeAsync(new SimpleStatement(POLL_CQL));

    $loop->watch($future);

    // Both handles report the same notifier descriptor, and a Context holds one
    // watcher per descriptor. The second add is rejected by the polling API.
    expect(fn () => $loop->watch($future))
        ->toThrow(Io\Poll\HandleAlreadyWatchedException::class);

    expect($loop->pending())->toBe(1);

    $returned = 0;
    while ($loop->pending() > 0) {
        $returned += count($loop->tick(10));
    }

    expect($returned)->toBe(1);
    $future->get();
})->group('feature', 'async', 'poll');
