# Data types

CQL has more numeric types than PHP. PHP has one integer type and one float type, so the driver uses
dedicated value classes wherever a plain PHP value would lose information or bind at the wrong
width.

All value classes live directly under `Cassandra\`. The sub-namespaces you see in the source tree
organize files, not classes.

```php
new Cassandra\Bigint('9223372036854775807');   // correct
new Cassandra\Numbers\Bigint(...);             // wrong, this class does not exist
```

## The mapping

| CQL type | Read as | Bind with |
| --- | --- | --- |
| `ascii`, `text`, `varchar` | `string` | `string` |
| `int` | `int` | `int` |
| `bigint`, `counter` | `Cassandra\Bigint` | `Cassandra\Bigint` |
| `smallint` | `Cassandra\Smallint` | `Cassandra\Smallint` |
| `tinyint` | `Cassandra\Tinyint` | `Cassandra\Tinyint` |
| `varint` | `Cassandra\Varint` | `Cassandra\Varint` |
| `decimal` | `Cassandra\Decimal` | `Cassandra\Decimal` |
| `float` | `Cassandra\Float` | `Cassandra\Float` |
| `double` | `float` | `float` |
| `boolean` | `bool` | `bool` |
| `blob` | `Cassandra\Blob` | `Cassandra\Blob` |
| `uuid` | `Cassandra\Uuid` | `Cassandra\Uuid` |
| `timeuuid` | `Cassandra\Timeuuid` | `Cassandra\Timeuuid` |
| `inet` | `Cassandra\Inet` | `Cassandra\Inet` |
| `timestamp` | `Cassandra\Timestamp` | `Cassandra\Timestamp` |
| `date` | `Cassandra\Date` | `Cassandra\Date` |
| `time` | `Cassandra\Time` | `Cassandra\Time` |
| `duration` | `Cassandra\Duration` | `Cassandra\Duration` |
| `list<T>` | `Cassandra\Collection` | `Cassandra\Collection` |
| `set<T>` | `Cassandra\Set` | `Cassandra\Set` |
| `map<K,V>` | `Cassandra\Map` | `Cassandra\Map` |
| `tuple<...>` | `Cassandra\Tuple` | `Cassandra\Tuple` |
| user defined type | `Cassandra\UserTypeValue` | `Cassandra\UserTypeValue` |
| any type, `NULL` value | `null` | `null` |

Five CQL types map to PHP natives in both directions: `text` family, `int`, `double`, `boolean`, and
`null`. Everything else uses a value class.

## The four traps

### 1. A plain int binds as a 4-byte int

```php
// Wrong. The server rejects it.
$session->execute($insert, ['arguments' => [$id, 42]]);
// Validation failed for type ... LongType: got 4 bytes

// Right.
$session->execute($insert, ['arguments' => [$id, new Cassandra\Bigint(42)]]);
```

A PHP `int` always binds as CQL `int`, which is 32 bits. A `bigint`, `counter`, `smallint`,
`tinyint`, or `varint` column needs its own class.

### 2. A plain float binds as a double

```php
// Wrong for a `float` column.
$session->execute($insert, ['arguments' => [$id, 1.5]]);

// Right.
$session->execute($insert, ['arguments' => [$id, new Cassandra\Float(1.5)]]);
```

A PHP `float` is a double. CQL `float` is 32-bit, so it needs `Cassandra\Float`.

### 3. Large integers arrive as strings

```php
$bigint = $row['login_count'];      // Cassandra\Bigint

$bigint->value();                   // string, exact
(string) $bigint;                   // the same string
$bigint->toInt();                   // int, may overflow on a 32-bit value range
```

`value()` returns a string because a `varint` or a `decimal` can exceed the range of a PHP `int`.
Convert only when you know the value fits.

### 4. A blob is not a string

```php
$blob = new Cassandra\Blob(random_bytes(16));

$blob->bytes();            // the raw bytes as a PHP string
$blob->toBinaryString();   // the raw bytes as a PHP string
(string) $blob;            // the hexadecimal form, for display
```

Binding a plain PHP string to a `blob` column binds it as text. Wrap it.

## Numbers

`Bigint`, `Smallint`, `Tinyint`, `Varint`, `Decimal`, and `Float` all implement
`Cassandra\Numeric`, so they share an arithmetic interface that preserves precision.

```php
$a = new Cassandra\Bigint('9007199254740993');
$b = new Cassandra\Bigint(2);

$a->add($b);      // Cassandra\Bigint
$a->sub($b);
$a->mul($b);
$a->div($b);
$a->mod($b);
$a->abs();
$a->neg();
$a->sqrt();

$a->toInt();      // int
$a->toDouble();   // float
(string) $a;      // exact decimal string
```

The constructors accept an `int`, a `float`, a numeric `string`, or another instance of the same
class. Pass a string for any value that a PHP `int` cannot hold exactly.

`Bigint`, `Smallint`, `Tinyint`, and `Float` also expose the range:

```php
Cassandra\Bigint::min();   // -9223372036854775808
Cassandra\Bigint::max();   //  9223372036854775807
```

`Decimal` keeps an unscaled value and a scale:

```php
$price = new Cassandra\Decimal('19.99');

$price->value();   // '1999' — the unscaled integer
$price->scale();   // 2
(string) $price;   // '19.99'
```

Division by zero raises `Cassandra\Exception\DivideByZeroException`.

`Float` reports the special values:

```php
$f = new Cassandra\Float(INF);

$f->isInfinite();   // true
$f->isFinite();     // false
$f->isNaN();        // false
```

## Dates and times

CQL splits date and time. The driver mirrors that split.

| Class | CQL type | Holds |
| --- | --- | --- |
| `Cassandra\Timestamp` | `timestamp` | A point in time, millisecond resolution |
| `Cassandra\Date` | `date` | A calendar day, no time and no zone |
| `Cassandra\Time` | `time` | A time of day, nanosecond resolution |
| `Cassandra\Duration` | `duration` | Months, days, and nanoseconds |

```php
$ts = new Cassandra\Timestamp(time());                 // from a Unix timestamp
$ts = Cassandra\Timestamp::fromDateTime(new DateTime());

$ts->time();                 // seconds
$ts->microtime(true);        // float
$ts->toDateTime();           // DateTime
```

```php
$date = new Cassandra\Date(time());
$date = Cassandra\Date::fromDateTime(new DateTimeImmutable('2026-08-05'));

$date->seconds();                       // Unix seconds at midnight
$date->toDateTime();                    // DateTime, midnight
$date->toDateTime(new Cassandra\Time(0)); // combine a date and a time
```

```php
$time = new Cassandra\Time(3_600_000_000_000);   // nanoseconds since midnight
$time = Cassandra\Time::fromDateTime(new DateTime('13:45:00'));

$time->seconds();
```

`Duration` is a calendar-aware interval, so it is not a fixed number of seconds. One month has a
different length depending on the month.

```php
$d = new Cassandra\Duration(months: 1, days: 15, nanos: 0);

$d->months();   // string
$d->days();     // string
$d->nanos();    // string
(string) $d;    // '1mo15d0ns'
```

The string form is `Mmo Dd Nns` without spaces. A negative duration leads with a minus sign, so
`(-3, -2, -1)` prints as `-3mo2d1ns`.

## UUIDs

```php
$id = new Cassandra\Uuid();                                      // random version 4
$id = new Cassandra\Uuid('7b5a4c1e-8f2a-4c9e-9a1b-3c2d1e0f5a6b'); // from a string

$id->uuid();      // canonical string
$id->version();   // 4
(string) $id;     // canonical string
```

A `timeuuid` embeds a timestamp and sorts by time. Use it as a clustering key for an event log.

```php
$tid = new Cassandra\Timeuuid();          // now
$tid = new Cassandra\Timeuuid(time());    // from a Unix timestamp

$tid->time();      // Unix seconds
$tid->version();   // 1
```

Both implement `Cassandra\UuidInterface`.

## IP addresses

```php
$ip = new Cassandra\Inet('192.168.1.1');
$ip = new Cassandra\Inet('2001:db8::1');

$ip->address();   // the string form
```

IPv4 and IPv6 both work. An invalid address raises
`Cassandra\Exception\InvalidArgumentException`.

## Type names in code

Every value has a `type()` method that returns a `Cassandra\Type`:

```php
$row['login_count']->type()->name();   // 'bigint'
(string) $row['tags']->type();         // 'set<text>'
```

The `Cassandra\Type` factory builds type objects, which collection constructors need. See
[collections and user defined types](/guide/collections) and the
[type factory reference](/reference/types).

## Comparison and equality

Value objects define their own comparison, so `==` compares the value, not the object identity:

```php
new Cassandra\Bigint(42) == new Cassandra\Bigint(42);   // true
new Cassandra\Bigint(42) === new Cassandra\Bigint(42);  // false, different objects
```

Use `==` for a value comparison. For a sort, compare the string form or use `toInt()` when the range
allows it.
