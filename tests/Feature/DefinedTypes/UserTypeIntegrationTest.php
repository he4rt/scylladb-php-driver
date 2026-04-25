<?php

declare(strict_types=1);

namespace Cassandra\Tests\Feature\DefinedTypes;

use Cassandra;
use Cassandra\Exception\InvalidQueryException;
use Cassandra\Exception\ServerException;
use Cassandra\Set;
use Cassandra\Timeuuid;
use Cassandra\Type;
use Cassandra\UserTypeValue;

$keyspace = 'udt_int_' . substr(bin2hex(random_bytes(4)), 0, 8);

beforeAll(function () use ($keyspace) {
    migrateKeyspace(<<<CQL
    CREATE KEYSPACE $keyspace WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};
    CREATE TYPE $keyspace.phone (alias text, number text);
    CREATE TYPE $keyspace.address (street text, zip int, phone_numbers set<frozen<phone>>);
    CREATE TABLE $keyspace.addresses (key timeuuid PRIMARY KEY, value frozen<address>);
    CREATE TABLE $keyspace.invalidphone (key int PRIMARY KEY, value frozen<phone>);
    CQL);
});

afterAll(fn () => dropKeyspace($keyspace));

function getPhoneUserType(): UserTypeValue
{
    return new UserTypeValue([
        'alias'  => Cassandra::TYPE_TEXT,
        'number' => Cassandra::TYPE_TEXT,
    ]);
}

function getAddressUserType(): UserTypeValue
{
    $phoneNumbers = new Set(getPhoneUserType()->type());
    return new UserTypeValue([
        'street'        => Cassandra::TYPE_TEXT,
        'zip'           => Cassandra::TYPE_INT,
        'phone_numbers' => $phoneNumbers->type(),
    ]);
}

function generateAddressValue(): UserTypeValue
{
    $home = getPhoneUserType();
    $home->set('alias', 'Home');
    $home->set('number', '555-911-1212');

    $work = getPhoneUserType();
    $work->set('alias', 'Work');
    $work->set('number', '650-389-6000');

    $numbers = new Set($home->type());
    $numbers->add($home);
    $numbers->add($work);

    $address = getAddressUserType();
    $address->set('street', '3975 Freedom Circle');
    $address->set('zip', 95054);
    $address->set('phone_numbers', $numbers);

    return $address;
}

function insertAddress(string $keyspace, UserTypeValue $address): Timeuuid
{
    $session = scyllaDbConnection($keyspace);
    $key = new Timeuuid();
    $session->execute(
        "INSERT INTO $keyspace.addresses (key, value) VALUES (?, ?)",
        ['arguments' => [$key, $address]]
    );
    return $key;
}

function selectAddress(string $keyspace, Timeuuid $key): UserTypeValue
{
    $session = scyllaDbConnection($keyspace);
    $rows = $session->execute(
        "SELECT value FROM $keyspace.addresses WHERE key=?",
        ['arguments' => [$key]]
    );

    expect($rows->count())->toBe(1);
    $row = $rows->first();
    expect($row)->not->toBeNull()
        ->and($row)->toHaveKey('value');

    $userType = $row['value'];
    expect($userType)->toBeInstanceOf(UserTypeValue::class);
    return $userType;
}

it('Round-trips a complete address user type value', function () use ($keyspace) {
    $address = generateAddressValue();
    $key = insertAddress($keyspace, $address);
    $stored = selectAddress($keyspace, $key);

    expect($stored->get('street'))->toBe('3975 Freedom Circle')
        ->and($stored->get('zip'))->toBe(95054);

    $numbers = $stored->get('phone_numbers');
    expect($numbers)->toBeInstanceOf(Set::class)
        ->and(count($numbers))->toBe(2);
});

it('Round-trips a partial user type with missing phone alias', function () use ($keyspace) {
    $phone = getPhoneUserType();
    $phone->set('number', '000-000-0000');
    $numbers = new Set($phone->type());
    $numbers->add($phone);

    $address = getAddressUserType();
    $address->set('street', '123 Missing Alias Street');
    $address->set('zip', 0);
    $address->set('phone_numbers', $numbers);

    $key = insertAddress($keyspace, $address);
    $stored = selectAddress($keyspace, $key);

    expect($stored->get('street'))->toBe('123 Missing Alias Street')
        ->and($stored->get('zip'))->toBe(0);
});

it('Stores a partial user type where most fields are null', function () use ($keyspace) {
    $address = getAddressUserType();
    $address->set('street', '1 Furzeground Way');
    $key = insertAddress($keyspace, $address);
    $stored = selectAddress($keyspace, $key);

    expect($stored->get('street'))->toBe('1 Furzeground Way')
        ->and($stored->get('zip'))->toBeNull()
        ->and($stored->get('phone_numbers'))->toBeNull();
});

it('Stores a null user type value', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);
    $key = new Timeuuid();
    $session->execute(
        "INSERT INTO $keyspace.addresses (key, value) VALUES (?, ?)",
        ['arguments' => [$key, null]]
    );
    $rows = $session->execute(
        "SELECT value FROM $keyspace.addresses WHERE key=?",
        ['arguments' => [$key]]
    );
    expect($rows->first()['value'])->toBeNull();
});

it('Throws when creating a non-frozen UDT (Cassandra < 3 only)', function () use ($keyspace) {
    if (version_compare(Cassandra::CPP_DRIVER_VERSION, '3.0.0') >= 0) {
        // ScyllaDB and Cassandra 3+ permit non-frozen UDTs in some contexts;
        // this test is only meaningful for legacy Cassandra 2.x.
        $this->markTestSkipped('Non-frozen UDT restriction does not apply on this server.');
    }

    $session = scyllaDbConnection($keyspace);
    expect(fn () => $session->execute(
        "CREATE TYPE $keyspace.frozen_required (id uuid, address address)"
    ))->toThrow(InvalidQueryException::class);
});

it('Throws InvalidQueryException when referencing an unknown user type', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);
    expect(fn () => $session->execute(
        "CREATE TABLE $keyspace.unavailable (id uuid PRIMARY KEY, unavailable frozen<user_type_unavailable>)"
    ))->toThrow(InvalidQueryException::class);
});

it('Throws ServerException when binding a phone where address is expected', function () use ($keyspace) {
    $session = scyllaDbConnection($keyspace);
    $invalid = generateAddressValue();

    // Insert an address-shaped UDT into a phone-typed column.
    expect(fn () => $session->execute(
        "INSERT INTO $keyspace.invalidphone (key, value) VALUES (?, ?)",
        ['arguments' => [1, $invalid]]
    ))->toThrow(ServerException::class);
});
