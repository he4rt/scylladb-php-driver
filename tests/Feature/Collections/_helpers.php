<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\Collections;

use Cassandra;
use Cassandra\Bigint;
use Cassandra\Blob;
use Cassandra\Date;
use Cassandra\Decimal;
use Cassandra\Duration;
use Cassandra\Inet;
use Cassandra\Smallint;
use Cassandra\Time;
use Cassandra\Timestamp;
use Cassandra\Timeuuid;
use Cassandra\Tinyint;
use Cassandra\Type;
use Cassandra\Uuid;
use Cassandra\Varint;

if (! function_exists(__NAMESPACE__ . '\\scalarCassandraTypes')) {

    /**
     * Scalar Cassandra type/value pairs (port of DatatypeIntegrationTests::scalarCassandraTypes).
     */
    function scalarCassandraTypes(): array
    {
        $floatClass = '\\Cassandra\\Float';
        return [
            [Type::ascii(),    ['a', 'b', 'c']],
            [Type::bigint(),   [new Bigint('1'), new Bigint('2'), new Bigint('3')]],
            [Type::blob(),     [new Blob('x'), new Blob('y'), new Blob('z')]],
            [Type::boolean(),  [true, false, true, false]],
            [Type::date(),     [new Date(), new Date(0), new Date(-86400), new Date(86400)]],
            [Type::decimal(),  [new Decimal(1.1), new Decimal(2.2), new Decimal(3.3)]],
            [Type::double(),   [1.1, 2.2, 3.3, 4.4]],
            [Type::duration(), [
                new Duration(0, 0, 0),
                new Duration(1, 2, 3),
                new Duration(1, 0, (2 ** 31) - 1),
                new Duration(-1, 0, -(2 ** 31)),
                new Duration((2 ** 31) - 1, 1, 0),
                new Duration(-(2 ** 31), -1, 0),
            ]],
            [Type::int(),      [1, 2, 99]],
            [Type::float(),    [new $floatClass(1.0), new $floatClass(2.2), new $floatClass(2.2)]],
            [Type::inet(),     [new Inet('127.0.0.1'), new Inet('127.0.0.2'), new Inet('127.0.0.3')]],
            [Type::smallint(), [Smallint::min(), Smallint::max(), new Smallint(0), new Smallint(74)]],
            [Type::text(),     ['a', 'b', 'c', 'x', 'y', 'z']],
            [Type::time(),     [new Time(), new Time(0), new Time(1234567890)]],
            [Type::tinyint(),  [Tinyint::min(), Tinyint::max(), new Tinyint(0), new Tinyint(37)]],
            [Type::timestamp(), [new Timestamp(123), new Timestamp(456), new Timestamp(789)]],
            [Type::timeuuid(), [new Timeuuid(0), new Timeuuid(1), new Timeuuid(2)]],
            [Type::uuid(),     [
                new Uuid('03398c99-c635-4fad-b30a-3b2c49f785c2'),
                new Uuid('03398c99-c635-4fad-b30a-3b2c49f785c3'),
                new Uuid('03398c99-c635-4fad-b30a-3b2c49f785c4'),
            ]],
            [Type::varchar(),  ['a', 'b', 'c', 'x', 'y', 'z']],
            [Type::varint(),   [new Varint(1), new Varint(2), new Varint(3)]],
        ];
    }

    /**
     * Constant Cassandra scalar types — pairs of (constant string, sample values).
     */
    function constantScalarCassandraTypes(): array
    {
        $constants = [
            Cassandra::TYPE_TEXT,
            Cassandra::TYPE_ASCII,
            Cassandra::TYPE_VARCHAR,
            Cassandra::TYPE_BIGINT,
            Cassandra::TYPE_SMALLINT,
            Cassandra::TYPE_TINYINT,
            Cassandra::TYPE_BLOB,
            Cassandra::TYPE_BOOLEAN,
            Cassandra::TYPE_DECIMAL,
            Cassandra::TYPE_DOUBLE,
            Cassandra::TYPE_FLOAT,
            Cassandra::TYPE_INT,
            Cassandra::TYPE_TIMESTAMP,
            Cassandra::TYPE_UUID,
            Cassandra::TYPE_VARINT,
            Cassandra::TYPE_TIMEUUID,
            Cassandra::TYPE_INET,
        ];
        $scalar = scalarCassandraTypes();

        return array_map(function ($type) use ($scalar) {
            $match = array_filter($scalar, fn ($item) => (string) $item[0] === $type);
            assert(count($match) > 0);
            return [$type, current($match)[1]];
        }, $constants);
    }

    /**
     * Composite Cassandra type/value pairs (collection, set, map, tuple, UDT).
     */
    function compositeCassandraTypes(): array
    {
        $collectionType = Type::collection(Type::varchar());
        $setType = Type::set(Type::varchar());
        $mapType = Type::map(Type::varchar(), Type::int());
        $tupleType = Type::tuple(Type::varchar(), Type::int(), Type::bigint());
        $userType = Type::userType('a', Type::varchar(), 'b', Type::int(), 'c', Type::bigint());
        $userType = $userType->withName(userTypeString($userType));

        return [
            [$collectionType, [
                $collectionType->create('a', 'b', 'c'),
                $collectionType->create('d', 'e', 'f'),
                $collectionType->create('x', 'y', 'z'),
            ]],
            [$setType, [
                $setType->create('a', 'b', 'c'),
                $setType->create('d', 'e', 'f'),
                $setType->create('x', 'y', 'z'),
            ]],
            [$mapType, [
                $mapType->create('a', 1, 'b', 2, 'c', 3),
                $mapType->create('d', 4, 'e', 5, 'f', 6),
                $mapType->create('x', 7, 'y', 8, 'z', 9),
            ]],
            [$tupleType, [
                $tupleType->create('a', 1, new Bigint(2)),
                $tupleType->create('b', 3, new Bigint(4)),
                $tupleType->create('c', 5, new Bigint(6)),
            ]],
            [$userType, [
                $userType->create('a', 'x', 'b', 1, 'c', new Bigint(2)),
                $userType->create('a', 'y', 'b', 3, 'c', new Bigint(4)),
                $userType->create('a', 'z', 'b', 5, 'c', new Bigint(6)),
            ]],
        ];
    }

    /**
     * Nested composite Cassandra type/value pairs.
     */
    function nestedCassandraTypes(): array
    {
        $composites = compositeCassandraTypes();
        $rows = [];

        foreach ($composites as $nested) {
            $type = Type::collection($nested[0]);
            $rows[] = [$type, [$type->create($nested[1][0])]];
        }
        foreach ($composites as $nested) {
            $type = Type::set($nested[0]);
            $rows[] = [$type, [$type->create($nested[1][0])]];
        }
        foreach ($composites as $nested) {
            $type = Type::map($nested[0], $nested[0]);
            $rows[] = [$type, [$type->create($nested[1][0], $nested[1][1])]];
        }
        foreach ($composites as $nested) {
            $type = Type::tuple($nested[0], $nested[0]);
            $rows[] = [$type, [$type->create($nested[1][0], $nested[1][1])]];
        }
        foreach ($composites as $nested) {
            $type = Type::userType('a', $nested[0], 'b', $nested[0]);
            $type = $type->withName(userTypeString($type));
            $rows[] = [$type, [$type->create('a', $nested[1][0], 'b', $nested[1][1])]];
        }
        return $rows;
    }

    function typeString($type): string
    {
        if ($type instanceof Type\Tuple || $type instanceof Type\Collection
            || $type instanceof Type\Map || $type instanceof Type\Set
            || $type instanceof Type\UserType) {
            return sprintf('frozen<%s>', $type);
        }
        return (string) $type;
    }

    function userTypeString(Type\UserType $userType): string
    {
        return implode('_', array_map(
            fn ($name, $type) => $name . str_replace(['frozen', '<', ' ', ',', '>'], '', $type),
            array_keys($userType->types()),
            $userType->types()
        ));
    }

    function collectionsEnsureUserTypes(string $keyspace, array $types): void
    {
        $session = \scyllaDbConnection($keyspace);
        foreach ($types as $cassandraType) {
            if ($cassandraType[0] instanceof Type\UserType) {
                $userType = $cassandraType[0];
                $fields = implode(', ', array_map(
                    fn ($name, $t) => $name . ' ' . typeString($t),
                    array_keys($userType->types()),
                    $userType->types()
                ));
                $session->execute(sprintf(
                    'CREATE TYPE IF NOT EXISTS %s.%s (%s)',
                    $keyspace,
                    userTypeString($userType),
                    $fields
                ));
            }
        }
    }

    function collectionsCreateTable(string $keyspace, $type): string
    {
        $session = \scyllaDbConnection($keyspace);
        $cqlType = typeString($type);
        $tableName = 'tbl_' . str_replace('-', '', (string) (new Uuid()));
        $session->execute(sprintf(
            'CREATE TABLE IF NOT EXISTS %s.%s (key text PRIMARY KEY, value %s)',
            $keyspace,
            $tableName,
            $cqlType
        ));
        return $tableName;
    }

    function collectionsCreateAndVerify(string $keyspace, $type, $value): void
    {
        // By index
        $tableName = collectionsCreateTable($keyspace, $type);
        $session = \scyllaDbConnection($keyspace);
        $key = 'key';
        $session->execute(
            "INSERT INTO $keyspace.$tableName (key, value) VALUES (?, ?)",
            ['arguments' => [$key, $value]]
        );
        collectionsVerifyValue($session, $keyspace, $tableName, $type, $key, $value);

        // By name
        $tableName = collectionsCreateTable($keyspace, $type);
        $session->execute(
            "INSERT INTO $keyspace.$tableName (key, value) VALUES (?, ?)",
            ['arguments' => ['key' => $key, 'value' => $value]]
        );
        collectionsVerifyValue($session, $keyspace, $tableName, $type, $key, $value);
    }

    function collectionsVerifyValue($session, string $keyspace, string $tableName, $type, string $key, $value): void
    {
        $result = $session->execute(
            "SELECT * FROM $keyspace.$tableName WHERE key = ?",
            ['arguments' => [$key]]
        );

        expect($result->count())->toEqual(1);
        $row = $result->first();
        expect($row['value'])->toEqual($value);

        if (isset($row['value']) && is_object($row['value'])) {
            expect($row['value']->type())->toEqual($type);
        }
    }
}
