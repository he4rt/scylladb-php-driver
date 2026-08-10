# Value classes

Every value class implements `Cassandra\Value`:

```php
interface Value
{
    public function type(): Type;
}
```

All of them live directly under `Cassandra\`. See [data types](/guide/data-types) for the CQL
mapping.

## Numbers

`Cassandra\Bigint`, `Cassandra\Smallint`, `Cassandra\Tinyint`, `Cassandra\Varint`,
`Cassandra\Decimal`, and `Cassandra\Float` implement `Cassandra\Numeric`:

```php
interface Numeric
{
    public function add(Numeric $num): Numeric;
    public function sub(Numeric $num): Numeric;
    public function mul(Numeric $num): Numeric;
    public function div(Numeric $num): Numeric;
    public function mod(Numeric $num): Numeric;
    public function abs(): Numeric;
    public function neg(): Numeric;
    public function sqrt(): Numeric;
    public function toInt(): int;
    public function toDouble(): float;
}
```

Each constructor accepts `int|float|string|<same class>`.

| Class | CQL | `value()` returns | `min()` / `max()` |
| --- | --- | --- | --- |
| `Cassandra\Bigint` | `bigint`, `counter` | `string` | yes |
| `Cassandra\Smallint` | `smallint` | `int` | yes |
| `Cassandra\Tinyint` | `tinyint` | `int` | yes |
| `Cassandra\Varint` | `varint` | `string` | no |
| `Cassandra\Decimal` | `decimal` | `string`, unscaled | no |
| `Cassandra\Float` | `float` | `float` | yes |

Extra methods:

```php
$decimal->scale();       // int

$float->isInfinite();    // bool
$float->isFinite();      // bool
$float->isNaN();         // bool
```

Division by zero raises `Cassandra\Exception\DivideByZeroException`.

## `Cassandra\Uuid` and `Cassandra\Timeuuid`

Both implement `Cassandra\UuidInterface`, which extends `Cassandra\Value`:

```php
interface UuidInterface extends Value
{
    public function uuid(): string;
    public function version(): int;
}
```

```php
final class Uuid implements Value, UuidInterface
{
    public function __construct(string $uuid = <random v4>) {}

    public function uuid(): string;
    public function version(): int;
    public function type(): Type;
    public function __toString(): string;
}
```

```php
final class Timeuuid implements Value, UuidInterface
{
    public function __construct(string|int $uuid = <now>) {}

    public function time(): int;     // Unix seconds
    public function uuid(): string;
    public function version(): int;
    public function type(): Type;
    public function __toString(): string;
}
```

## Date and time

```php
final class Timestamp implements Value
{
    // Omit both arguments to get the current time.
    // A DateTimeInterface cannot be combined with $microseconds.
    public function __construct(int|\DateTimeInterface $seconds = <now>, int $microseconds = <now>) {}

    public static function now(): static;
    public static function nowUtc(): static;                      // alias of now()
    public static function fromDateTime(\DateTimeInterface $datetime): static;
    public function toDateTime(): \DateTime;
    public function time(): int;                                  // seconds
    public function microtime(bool $get_as_float = false): float|string;
    public function type(): Type;
    public function __toString(): string;
}
```

```php
final class Date implements Value
{
    public function __construct(int|string $value = <today>) {}

    public static function fromDateTime(\DateTimeInterface $datetime): static;
    public function toDateTime(?Time $time = null): \DateTime;
    public function seconds(): int;
    public function type(): Type;
    public function __toString(): string;
}
```

```php
final class Time implements Value
{
    public function __construct(int|string $nanoseconds = <now>) {}

    public static function fromDateTime(\DateTimeInterface $datetime): static;
    public function seconds(): int;
    public function type(): Type;
    public function __toString(): string;
}
```

```php
final class Duration implements Value
{
    // A DateInterval must be the only argument. Otherwise all three are required.
    public function __construct(
        int|float|string|Bigint|\DateInterval $months,
        int|float|string|Bigint $days,
        int|float|string|Bigint $nanos,
    ) {}

    public static function fromDateInterval(\DateInterval $interval): static;
    public function months(): string;
    public function days(): string;
    public function nanos(): string;
    public function type(): Type;
    public function __toString(): string;   // '1mo15d0ns'
}
```

## `Cassandra\Blob`

```php
final class Blob implements Value
{
    public function __construct(string $bytes) {}

    public function bytes(): string;            // raw bytes
    public function toBinaryString(): string;   // raw bytes
    public function type(): Type;
    public function __toString(): string;       // hexadecimal, for display
}
```

## `Cassandra\Inet`

```php
final class Inet implements Value
{
    public function __construct(string $address) {}   // IPv4 or IPv6

    public function address(): string;
    public function type(): Type;
    public function __toString(): string;
}
```

## `Cassandra\Collection`

CQL `list<T>`. Implements `Value`, `Countable`, and `Iterator`.

```php
public function __construct(Cassandra\Type|string $type) {}

public function add(mixed ...$value): int;      // the new count
public function get(int $index): mixed;
public function find(mixed $value): int|null;
public function remove(int $index): bool;
public function values(): array;
public function type(): Type;
```

## `Cassandra\Set`

CQL `set<T>`. Implements `Value`, `Countable`, and `Iterator`.

```php
public function __construct(Cassandra\Type|string $type) {}

public function add(mixed $value): bool;        // false when already present
public function has(mixed $value): bool;
public function remove(mixed $value): bool;
public function values(): array;
public function type(): Type;
```

## `Cassandra\Map`

CQL `map<K,V>`. Implements `Value`, `Countable`, `Iterator`, and `ArrayAccess`.

```php
public function __construct(Cassandra\Type|string $keyType, Cassandra\Type|string $valueType) {}

public function set(mixed $key, mixed $value): bool;
public function get(mixed $key): mixed;
public function has(mixed $key): bool;
public function remove(mixed $key): bool;
public function keys(): array;
public function values(): array;
public function type(): Type;
```

`ArrayAccess` works too: `$map['k'] = $v`, `$map['k']`, `isset($map['k'])`, `unset($map['k'])`.

## `Cassandra\Tuple`

CQL `tuple<...>`. Implements `Value`, `Countable`, and `Iterator`.

```php
public function __construct(array $types) {}

public function set(int $index, mixed $value): void;
public function get(int $index): mixed;
public function values(): array;
public function type(): Type;
```

## `Cassandra\UserTypeValue`

A user defined type value. Implements `Value`, `Countable`, and `Iterator`.

```php
public function __construct(array $types) {}   // ['field' => type, ...]

public function set(string $name, mixed $value): void;
public function get(string $name): mixed;
public function values(): array;
public function type(): Type;
```

Building it from the schema keeps the field types in step with the cluster:

```php
$type  = $session->schema()->keyspace('shop')->userType('address');
$value = $type->create('city', 'Paris', 'country', 'FR');
```

See [collections and user defined types](/guide/collections).

## Comparison

Value objects define their own comparison, so `==` compares by value:

```php
new Cassandra\Bigint(42) == new Cassandra\Bigint(42);    // true
new Cassandra\Bigint(42) === new Cassandra\Bigint(42);   // false
```
