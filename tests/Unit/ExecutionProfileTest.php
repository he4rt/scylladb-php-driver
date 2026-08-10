<?php

declare(strict_types=1);

namespace Cassandra\Tests\Unit;

use Cassandra;

enum StringBackedProfile: string
{
    case Analytics = 'analytics';
}

enum IntBackedProfile: int
{
    case Fast = 1;
}

enum PureProfile
{
    case Reporting;
}

/**
 * Comparing two Cluster objects proves nothing about caching: the compare
 * handler compares object handles, so distinct objects never match whatever
 * the cache did. This counter is the only honest signal. Give every case its
 * own profile name, or an earlier case's entry turns the first build into a
 * cache hit and hides the result.
 */
function cachedClusters(): int
{
    ob_start();
    phpinfo(INFO_MODULES);
    $info = (string) ob_get_clean();
    preg_match('/Persistent Clusters => (\d+)/', $info, $m);

    return (int) ($m[1] ?? -1);
}

describe('Cassandra\ExecutionProfile', function () {

    it('chains every setter and stays the same object', function () {
        $profile = new Cassandra\ExecutionProfile();

        expect($profile->withConsistency(Cassandra::CONSISTENCY_LOCAL_ONE))->toBe($profile)
            ->and($profile->withSerialConsistency(Cassandra::CONSISTENCY_LOCAL_SERIAL))->toBe($profile)
            ->and($profile->withRequestTimeout(2.5))->toBe($profile)
            ->and($profile->withRetryPolicy(new Cassandra\RetryPolicy\Fallthrough()))->toBe($profile)
            ->and($profile->withRoundRobinLoadBalancingPolicy())->toBe($profile)
            ->and($profile->withDatacenterAwareRoundRobinLoadBalancingPolicy('dc1', 0, false))->toBe($profile)
            ->and($profile->withTokenAwareRouting(true))->toBe($profile)
            ->and($profile->withTokenAwareRoutingShuffleReplicas(false))->toBe($profile)
            ->and($profile->withLatencyAwareRouting(true))->toBe($profile)
            ->and($profile->withLatencyAwareRoutingSettings(2.0, 0.1, 10.0, 0.1, 50))->toBe($profile)
            ->and($profile->withConstantSpeculativeExecutionPolicy(0.2, 3))->toBe($profile)
            ->and($profile->withNoSpeculativeExecutionPolicy())->toBe($profile)
            ->and($profile->withWhiteListHosts('10.0.0.1', '10.0.0.2'))->toBe($profile)
            ->and($profile->withBlackListHosts('10.0.0.9'))->toBe($profile)
            ->and($profile->withWhiteListDCs('dc1'))->toBe($profile)
            ->and($profile->withBlackListDCs('dc2'))->toBe($profile);
    });

    it('rejects a consistency that is not a consistency', function () {
        expect(fn () => (new Cassandra\ExecutionProfile())->withConsistency(999))
            ->toThrow(Cassandra\Exception\InvalidArgumentException::class);
    });

    it('rejects a serial consistency that is not serial', function () {
        expect(fn () => (new Cassandra\ExecutionProfile())->withSerialConsistency(Cassandra::CONSISTENCY_ONE))
            ->toThrow(Cassandra\Exception\InvalidArgumentException::class);

        expect((new Cassandra\ExecutionProfile())->withSerialConsistency(Cassandra::CONSISTENCY_SERIAL))
            ->toBeInstanceOf(Cassandra\ExecutionProfile::class);
    });

    it('rejects a negative request timeout and speculative delay', function () {
        expect(fn () => (new Cassandra\ExecutionProfile())->withRequestTimeout(-1.0))
            ->toThrow(Cassandra\Exception\InvalidArgumentException::class);

        expect(fn () => (new Cassandra\ExecutionProfile())->withConstantSpeculativeExecutionPolicy(0.0))
            ->toThrow(Cassandra\Exception\InvalidArgumentException::class);
    });

    describe('attaching to a cluster', function () {

        it('accepts several named profiles', function () {
            $cluster = Cassandra::cluster()
                ->withExecutionProfile('analytics', (new Cassandra\ExecutionProfile())
                    ->withConsistency(Cassandra::CONSISTENCY_LOCAL_ONE)
                    ->withRequestTimeout(30.0))
                ->withExecutionProfile('oltp', (new Cassandra\ExecutionProfile())
                    ->withConsistency(Cassandra::CONSISTENCY_LOCAL_QUORUM))
                ->withPersistentSessions(false)
                ->build();

            expect($cluster)->toBeInstanceOf(Cassandra\Cluster::class);
        });

        it('rejects an empty profile name', function () {
            expect(fn () => Cassandra::cluster()->withExecutionProfile('', new Cassandra\ExecutionProfile()))
                ->toThrow(Cassandra\Exception\InvalidArgumentException::class);
        });

        /*
         * The persistent-cluster cache key is a hash of the builder fields, and
         * CassExecProfile is opaque. Without the profile fingerprint in that key,
         * two clusters differing only in a profile would share one CassCluster
         * and silently get the wrong settings.
         */

        it('does not share a cached cluster between differing profiles', function () {
            $before = cachedClusters();

            $make = fn (int $consistency) => Cassandra::cluster()
                ->withExecutionProfile('differing-profiles', (new Cassandra\ExecutionProfile())
                    ->withConsistency($consistency))
                ->build();

            $a = $make(Cassandra::CONSISTENCY_ONE);
            $b = $make(Cassandra::CONSISTENCY_ALL);

            expect(cachedClusters() - $before)->toBe(2);
        });

        it('shares a cached cluster when the profiles match', function () {
            $before = cachedClusters();

            $make = fn () => Cassandra::cluster()
                ->withExecutionProfile('matching-profiles', (new Cassandra\ExecutionProfile())
                    ->withConsistency(Cassandra::CONSISTENCY_ONE))
                ->build();

            $a = $make();
            $b = $make();

            expect(cachedClusters() - $before)->toBe(1);
        });
    });

    describe('naming a profile with an enum', function () {

        it('registers under the same name whether given the enum or its value', function () {
            $before = cachedClusters();

            $make = fn (string|\UnitEnum $name) => Cassandra::cluster()
                ->withExecutionProfile($name, (new Cassandra\ExecutionProfile())
                    ->withConsistency(Cassandra::CONSISTENCY_TWO))
                ->build();

            $a = $make(StringBackedProfile::Analytics);
            $b = $make('analytics');

            expect(cachedClusters() - $before)->toBe(1);
        });

        it('uses the case name of a pure enum, not a lowercased one', function () {
            $before = cachedClusters();

            $make = fn (string|\UnitEnum $name) => Cassandra::cluster()
                ->withExecutionProfile($name, (new Cassandra\ExecutionProfile())
                    ->withConsistency(Cassandra::CONSISTENCY_THREE))
                ->build();

            $a = $make(PureProfile::Reporting);
            $b = $make('Reporting');

            expect(cachedClusters() - $before)->toBe(1);
        });

        it('uses the case name of an int-backed enum, since a number is no name', function () {
            $before = cachedClusters();

            $make = fn (string|\UnitEnum $name) => Cassandra::cluster()
                ->withExecutionProfile($name, (new Cassandra\ExecutionProfile())
                    ->withConsistency(Cassandra::CONSISTENCY_QUORUM))
                ->build();

            $a = $make(IntBackedProfile::Fast);
            $b = $make('Fast');

            expect(cachedClusters() - $before)->toBe(1);
        });

        it('keeps a string-backed enum distinct from its case name', function () {
            $before = cachedClusters();

            $make = fn (string|\UnitEnum $name) => Cassandra::cluster()
                ->withExecutionProfile($name, (new Cassandra\ExecutionProfile())
                    ->withConsistency(Cassandra::CONSISTENCY_ALL))
                ->build();

            $a = $make(StringBackedProfile::Analytics);   // "analytics", the value
            $b = $make('Analytics');                      // the case name

            expect(cachedClusters() - $before)->toBe(2);
        });

        it('rejects a name that is neither a string nor an enum', function () {
            expect(fn () => Cassandra::cluster()->withExecutionProfile(123, new Cassandra\ExecutionProfile()))
                ->toThrow(Cassandra\Exception\InvalidArgumentException::class);
        });
    });

    describe('selecting a profile when executing', function () {

        it('takes the profile as a third argument, not an option key', function () {
            $execute = new \ReflectionMethod(Cassandra\DefaultSession::class, 'execute');
            $async   = new \ReflectionMethod(Cassandra\DefaultSession::class, 'executeAsync');

            foreach ([$execute, $async] as $method) {
                $param = $method->getParameters()[2];

                expect($param->getName())->toBe('executionProfile')
                    ->and((string) $param->getType())->toBe('UnitEnum|string|null')
                    ->and($param->isOptional())->toBeTrue()
                    ->and($param->getDefaultValue())->toBeNull();
            }
        });

        it('is declared on the Session interface too', function () {
            $param = (new \ReflectionMethod(Cassandra\Session::class, 'execute'))->getParameters()[2];

            expect($param->getName())->toBe('executionProfile')
                ->and((string) $param->getType())->toBe('UnitEnum|string|null');
        });
    });

});
