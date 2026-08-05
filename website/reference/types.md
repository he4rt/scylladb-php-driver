# Type factory

`Cassandra\Type` is an abstract class with static factory methods. It builds the type objects that
collection constructors need, and it is what `Value::type()` returns.

```php
abstract class Type
{
    abstract public function name(): string|null;
    abstract public function __toString(): string;
}
```

## Scalar types

Each returns a `Cassandra\Type\Scalar`.

```php
Cassandra\Type::ascii();
Cassandra\Type::text();
Cassandra\Type::varchar();

Cassandra\Type::tinyint();
Cassandra\Type::smallint();
Cassandra\Type::int();
Cassandra\Type::bigint();
Cassandra\Type::varint();
Cassandra\Type::counter();

Cassandra\Type::float();
Cassandra\Type::double();
Cassandra\Type::decimal();

Cassandra\Type::boolean();
Cassandra\Type::blob();
Cassandra\Type::inet();

Cassandra\Type::uuid();
Cassandra\Type::timeuuid();

Cassandra\Type::timestamp();
Cassandra\Type::date();
Cassandra\Type::time();
Cassandra\Type::duration();
```

## Composite types

```php
Cassandra\Type::collection(Type $type): Cassandra\Type\Collection;   // list<T>
Cassandra\Type::set(Type $type): Cassandra\Type\Set;
Cassandra\Type::map(Type $keyType, Type $valueType): Cassandra\Type\Map;
Cassandra\Type::tuple(Type ...$types): Cassandra\Type\Tuple;
Cassandra\Type::userType(mixed ...$types): Cassandra\Type\UserType;
```

```php
$listOfText  = Cassandra\Type::collection(Cassandra\Type::text());
$setOfUuid   = Cassandra\Type::set(Cassandra\Type::uuid());
$mapTextInt  = Cassandra\Type::map(Cassandra\Type::text(), Cassandra\Type::int());
$coordinates = Cassandra\Type::tuple(
    Cassandra\Type::double(),
    Cassandra\Type::double(),
);
```

`userType()` takes alternating field names and types:

```php
$address = Cassandra\Type::userType(
    'street',  Cassandra\Type::text(),
    'city',    Cassandra\Type::text(),
    'country', Cassandra\Type::text(),
)
    ->withName('address')
    ->withKeyspace('shop');
```

Reading the type from the schema is better, because it stays in step with the cluster:

```php
$address = $session->schema()->keyspace('shop')->userType('address');
```

## Creating values from a type

Every type object has a `create()` method, which is often shorter than the value constructor.

```php
Cassandra\Type\Scalar::create(mixed $value = null): mixed;
Cassandra\Type\Collection::create(mixed ...$value): Cassandra\Collection;
Cassandra\Type\Set::create(mixed ...$value): Cassandra\Set;
Cassandra\Type\Map::create(mixed ...$value): Cassandra\Map;        // key, value, key, value
Cassandra\Type\Tuple::create(mixed ...$values): Cassandra\Tuple;
Cassandra\Type\UserType::create(mixed ...$value): Cassandra\UserTypeValue;  // name, value, name, value
```

```php
$tags   = Cassandra\Type::collection(Cassandra\Type::text())->create('php', 'cql');
$roles  = Cassandra\Type::set(Cassandra\Type::text())->create('admin', 'editor');
$attrs  = Cassandra\Type::map(Cassandra\Type::text(), Cassandra\Type::int())
    ->create('logins', 12, 'visits', 340);
$coords = Cassandra\Type::tuple(Cassandra\Type::double(), Cassandra\Type::double())
    ->create(48.8584, 2.2945);
```

`Map::create()` and `UserType::create()` take pairs. An odd number of arguments raises
`Cassandra\Exception\InvalidArgumentException`.

`Cassandra\Type\Custom::create()` always throws. A custom type cannot be built on the client.

## Type name strings

Collection constructors also accept the type name as a string, which is shorter for a scalar
element type.

```php
new Cassandra\Collection(Cassandra::TYPE_TEXT);
new Cassandra\Map(Cassandra::TYPE_TEXT, Cassandra::TYPE_INT);
```

The constants are listed in the [constants reference](/reference/constants#cql-type-names). A
composite element type needs a real type object, not a string.

## Inspecting a type

```php
$type = $row['tags']->type();

$type->name();       // 'set'
(string) $type;      // 'set<text>'
```

| Class | Extra methods |
| --- | --- |
| `Cassandra\Type\Scalar` | — |
| `Cassandra\Type\Collection` | `valueType()` |
| `Cassandra\Type\Set` | `valueType()` |
| `Cassandra\Type\Map` | `keyType()`, `valueType()` |
| `Cassandra\Type\Tuple` | `types()` |
| `Cassandra\Type\UserType` | `types()`, `name()`, `keyspace()`, `withName()`, `withKeyspace()` |
| `Cassandra\Type\Custom` | `name()` |

```php
$mapType = $row['attributes']->type();

$mapType->keyType();     // Cassandra\Type\Scalar, 'text'
$mapType->valueType();   // Cassandra\Type\Scalar, 'int'
```

All type classes except `Cassandra\Type` itself have a private constructor. Use the factory methods.
