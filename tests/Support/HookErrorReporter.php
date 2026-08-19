<?php

declare(strict_types=1);

namespace Cassandra\Tests\Support;

use PHPUnit\Event\Test\AfterLastTestMethodErrored;
use PHPUnit\Event\Test\AfterLastTestMethodErroredSubscriber;
use PHPUnit\Event\Test\BeforeFirstTestMethodErrored;
use PHPUnit\Event\Test\BeforeFirstTestMethodErroredSubscriber;
use PHPUnit\Runner\Extension\Extension;
use PHPUnit\Runner\Extension\Facade;
use PHPUnit\Runner\Extension\ParameterCollection;
use PHPUnit\TextUI\Configuration\Configuration;

/**
 * Prints the exception when a beforeAll() or afterAll() hook throws.
 *
 * PHPUnit counts such an exception as an error, and the error check runs last
 * in ShellExitCodeCalculator, so it sets the shell exit code to 2 whatever
 * else happened. Pest's printer shows neither the exception nor an error
 * count: the run prints a summary that says every test passed and then exits 2
 * with no reason anywhere in the output. Only --debug shows it, which is too
 * loud for CI.
 *
 * The hooks in tests/Feature run keyspace DDL, so a flaky cluster fails the
 * whole run and leaves nothing to say which hook broke.
 *
 * Each subscriber is its own class on purpose. PHPUnit binds a subscriber
 * object to one event type, so a single class implementing several subscriber
 * interfaces receives only one of them.
 */
final class HookErrorReporter implements Extension
{
    public function bootstrap(Configuration $configuration, Facade $facade, ParameterCollection $parameters): void
    {
        $facade->registerSubscribers(
            new BeforeAllErrorSubscriber(),
            new AfterAllErrorSubscriber(),
        );
    }

    public static function report(string $hook, string $class, string $method, object $throwable): void
    {
        fwrite(STDERR, sprintf(
            "\n%s\nHOOK ERROR — the run exits with code 2. The summary does not show this.\n"
            . "  hook:      %s (%s::%s)\n  exception: %s\n  message:   %s\n%s\n%s\n",
            str_repeat('=', 78),
            $hook,
            $class,
            $method,
            $throwable->className(),
            trim($throwable->message()),
            rtrim($throwable->stackTrace()),
            str_repeat('=', 78),
        ));
    }
}

final class BeforeAllErrorSubscriber implements BeforeFirstTestMethodErroredSubscriber
{
    public function notify(BeforeFirstTestMethodErrored $event): void
    {
        HookErrorReporter::report(
            'beforeAll',
            $event->testClassName(),
            $event->calledMethod()->methodName(),
            $event->throwable(),
        );
    }
}

final class AfterAllErrorSubscriber implements AfterLastTestMethodErroredSubscriber
{
    public function notify(AfterLastTestMethodErrored $event): void
    {
        HookErrorReporter::report(
            'afterAll',
            $event->testClassName(),
            $event->calledMethod()->methodName(),
            $event->throwable(),
        );
    }
}
