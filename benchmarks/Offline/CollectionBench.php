<?php

declare(strict_types=1);

namespace Cassandra\Benchmarks\Offline;

use Cassandra;
use Cassandra\Collection;
use Cassandra\Map;
use Cassandra\Set;
use Cassandra\Tuple;
use Cassandra\UserTypeValue;
use Cassandra\Benchmarks\Support\Sizes;
use PhpBench\Attributes as Bench;

/**
 * Collection marshalling as a function of element count.
 *
 * Building a Set/Map/List means one native append per element plus dedup for
 * sets; reading `values()` materialises a PHP array. Both are on the hot path
 * for any query that binds or returns a collection column, and both should
 * scale linearly — the parameterised sets make a super-linear regression
 * obvious at a glance.
 */
#[Bench\Groups(['offline', 'collection'])]
#[Bench\Iterations(5)]
#[Bench\Warmup(2)]
#[Bench\ParamProviders('provideElementCounts')]
#[Bench\OutputTimeUnit('microseconds', precision: 3)]
final class CollectionBench
{
    use Sizes;

    #[Bench\Revs(200)]
    public function benchSetBuild(array $params): void
    {
        $set = new Set(Cassandra::TYPE_INT);
        for ($i = 0, $n = $params['count']; $i < $n; $i++) {
            $set->add($i);
        }
    }

    #[Bench\Revs(200)]
    public function benchSetBuildAndRead(array $params): void
    {
        $set = new Set(Cassandra::TYPE_INT);
        for ($i = 0, $n = $params['count']; $i < $n; $i++) {
            $set->add($i);
        }
        $set->values();
    }

    #[Bench\Revs(200)]
    public function benchMapBuild(array $params): void
    {
        $map = new Map(Cassandra::TYPE_TEXT, Cassandra::TYPE_INT);
        for ($i = 0, $n = $params['count']; $i < $n; $i++) {
            $map->set('k' . $i, $i);
        }
    }

    #[Bench\Revs(200)]
    public function benchListBuild(array $params): void
    {
        $list = new Collection(Cassandra::TYPE_TEXT);
        for ($i = 0, $n = $params['count']; $i < $n; $i++) {
            $list->add('v' . $i);
        }
    }

    #[Bench\Revs(200)]
    public function benchTupleBuild(array $params): void
    {
        // Tuples are fixed-arity; scale by repeating a 2-field tuple $count times.
        for ($i = 0, $n = $params['count']; $i < $n; $i++) {
            $tuple = new Tuple([Cassandra::TYPE_INT, Cassandra::TYPE_TEXT]);
            $tuple->set(0, $i);
            $tuple->set(1, 'x');
        }
    }

    #[Bench\Revs(200)]
    public function benchUdtBuild(array $params): void
    {
        for ($i = 0, $n = $params['count']; $i < $n; $i++) {
            $udt = new UserTypeValue([
                'id'   => Cassandra::TYPE_INT,
                'name' => Cassandra::TYPE_TEXT,
            ]);
            $udt->set('id', $i);
            $udt->set('name', 'x');
        }
    }
}
