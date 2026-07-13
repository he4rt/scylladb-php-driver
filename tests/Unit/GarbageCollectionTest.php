<?php

declare(strict_types=1);

use Cassandra\Type;
use Cassandra\Decimal;
use Cassandra\Varint;

/*
 * Cycle-collector smoke coverage for the object handlers whose get_gc used to
 * fall back to zend_std_get_gc.
 *
 * Those objects keep their data (element values, sub-types, the value's type)
 * in the C struct, not in PHP properties. The default get_gc invoked a
 * get_properties that *rebuilds* object->properties on every call; because the
 * cycle collector calls get_gc several times per run, it decremented one set of
 * child refcounts and re-incremented a freshly rebuilt set, corrupting
 * refcounts and eventually freeing a shared type's CassDataType (a crash on
 * teardown). The fix gives each handler a real zend_get_gc_buffer over its
 * actual C-struct zvals.
 *
 * NOTE ON SCOPE: this test exercises every fixed get_gc path through
 * gc_collect_cycles() and asserts the objects stay correct, but it is NOT a
 * standalone reproducer — the original fault only surfaced after the cumulative
 * allocation churn of the full suite. The definitive regression guard is to run
 * the whole test suite under AddressSanitizer (USE_ZEND_ALLOC=0 + the ASan
 * runtime): it was red before the fix and clean after. Keep that in CI.
 */

it('runs every fixed collection/type get_gc path through the collector', function () {
    $intType   = Type::int();
    $setType   = Type::set($intType);
    $tupleType = Type::tuple($intType, Type::varchar(), $setType);

    for ($i = 0; $i < 200; $i++) {
        $values = [
            $setType->create(1, 2, 3),
            Type::map(Type::varchar(), $intType)->create('a', 1, 'b', 2),
            Type::collection($intType)->create(4, 5, 6),
            $tupleType->create(9, 'x', $setType->create(7, 8)),
        ];

        foreach ($values as $v) {
            // Materialise object->properties — what the old get_gc rebuilt.
            $props = (array) $v;
            $dump  = print_r($v, true);
            unset($props, $dump);
        }

        // Trap the values and the shared types in a reference cycle, then drop
        // all strong refs so only the collector can reclaim them.
        $cycle = new stdClass();
        $cycle->self = $cycle;
        $cycle->values = $values;
        $cycle->types = [$intType, $setType, $tupleType];
        unset($values, $cycle);

        gc_collect_cycles();
    }

    // The shared types survive the churn and still produce correct values.
    expect($setType->create(1)->values())->toBe([1])
        ->and($tupleType->create(1, 'y', $setType->create(2))->get(0))->toBe(1);
})->group('unit', 'gc');

it('runs Decimal/Varint and cluster-builder get_gc paths through the collector', function () {
    for ($i = 0; $i < 200; $i++) {
        $objs = [
            new Decimal('3.14159265358979323846'),
            new Varint('123456789012345678901234567890'),
            Cassandra::cluster()->withDefaultTimeout(12.5)->withContactPoints('127.0.0.1'),
        ];

        foreach ($objs as $o) {
            $props = (array) $o;
            unset($props);
        }

        $cycle = new stdClass();
        $cycle->self = $cycle;
        $cycle->objs = $objs;
        unset($objs, $cycle);

        gc_collect_cycles();
    }

    expect((string) new Decimal('42'))->toBe('42')
        ->and((string) new Varint('42'))->toBe('42');
})->group('unit', 'gc');
