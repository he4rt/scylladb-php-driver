# Collections and user defined types

CQL has four composite types. Each maps to a PHP class that carries its element type, because the
driver must know the CQL type to encode the values.

| CQL | PHP class | Ordered | Duplicates | Keyed |
| --- | --- | --- | --- | --- |
| `list<T>` | `Cassandra\Collection` | yes | yes | no |
| `set<T>` | `Cassandra\Set` | no | no | no |
| `map<K,V>` | `Cassandra\Map` | no | no | yes |
| `tuple<A,B,…>` | `Cassandra\Tuple` | yes, fixed | — | by position |
| user defined type | `Cassandra\UserTypeValue` | — | — | by field name |

All of them implement `Countable` and `Iterator`. `Map` also implements `ArrayAccess`.

## Lists

```sql
CREATE TABLE posts (id uuid PRIMARY KEY, tags list<text>);
```

```php
$tags = new Cassandra\Collection(Cassandra::TYPE_TEXT);

$tags->add('php');                 // returns the new count
$tags->add('scylladb', 'cql');     // variadic

$tags->get(0);           // 'php'
$tags->find('cql');      // 2, or null when absent
$tags->remove(0);        // true
count($tags);            // 2
$tags->values();         // a plain PHP array

$session->execute($insert, ['arguments' => [$id, $tags]]);
```

The constructor takes a type name string or a `Cassandra\Type`:

```php
new Cassandra\Collection(Cassandra::TYPE_TEXT);
new Cassandra\Collection(Cassandra\Type::text());
```

## Sets

```sql
CREATE TABLE users (id uuid PRIMARY KEY, roles set<text>);
```

```php
$roles = new Cassandra\Set(Cassandra::TYPE_TEXT);

$roles->add('admin');      // true when added, false when already present
$roles->add('admin');      // false
$roles->has('admin');      // true
$roles->remove('admin');   // true
$roles->values();          // a plain PHP array
```

A set holds each value once and does not keep insertion order. Use it for membership. Use a list
when order matters.

## Maps

```sql
CREATE TABLE users (id uuid PRIMARY KEY, attributes map<text, int>);
```

```php
$attrs = new Cassandra\Map(Cassandra::TYPE_TEXT, Cassandra::TYPE_INT);

$attrs->set('logins', 12);
$attrs->get('logins');       // 12
$attrs->has('logins');       // true
$attrs->remove('logins');    // true

// ArrayAccess works as well:
$attrs['logins'] = 12;
echo $attrs['logins'];
unset($attrs['logins']);

$attrs->keys();     // a plain PHP array
$attrs->values();   // a plain PHP array
```

A map key can be any CQL type, including a value class:

```php
$byId = new Cassandra\Map(Cassandra::TYPE_UUID, Cassandra::TYPE_TEXT);
$byId->set(new Cassandra\Uuid(), 'ada');
```

That is why `Map` does not simply take a PHP array. A PHP array key can only be an int or a string.

## Tuples

A tuple is a fixed-length, positional group with a type per position.

```sql
CREATE TABLE places (id uuid PRIMARY KEY, coords tuple<double, double, text>);
```

```php
$coords = new Cassandra\Tuple([
    Cassandra::TYPE_DOUBLE,
    Cassandra::TYPE_DOUBLE,
    Cassandra::TYPE_TEXT,
]);

$coords->set(0, 48.8584);
$coords->set(1, 2.2945);
$coords->set(2, 'Paris');

$coords->get(0);     // 48.8584
$coords->values();   // [48.8584, 2.2945, 'Paris']
count($coords);      // 3
```

Build one from a type object instead, which is shorter:

```php
$type = Cassandra\Type::tuple(
    Cassandra\Type::double(),
    Cassandra\Type::double(),
    Cassandra\Type::text(),
);

$coords = $type->create(48.8584, 2.2945, 'Paris');
```

## User defined types

```sql
CREATE TYPE shop.address (
    street text,
    city text,
    zip text,
    country text
);

CREATE TABLE shop.users (
    id uuid PRIMARY KEY,
    home frozen<address>
);
```

### Building a value

```php
$address = new Cassandra\UserTypeValue([
    'street'  => Cassandra::TYPE_TEXT,
    'city'    => Cassandra::TYPE_TEXT,
    'zip'     => Cassandra::TYPE_TEXT,
    'country' => Cassandra::TYPE_TEXT,
]);

$address->set('street', '1 Rue de Rivoli');
$address->set('city', 'Paris');
$address->set('zip', '75001');
$address->set('country', 'FR');

$session->execute($insert, ['arguments' => [$id, $address]]);
```

### Building from the schema

Declaring the field types by hand duplicates the schema. Read the real type from the cluster
instead:

```php
$type = $session->schema()
    ->keyspace('shop')
    ->userType('address');

$address = $type->create(
    'street',  '1 Rue de Rivoli',
    'city',    'Paris',
    'zip',     '75001',
    'country', 'FR',
);
```

`create()` takes alternating field names and values. The type stays in sync with the schema, so a
new field does not break the code. See [schema metadata](/guide/schema-metadata).

### Reading a value

```php
$row  = $session->execute($findById, ['arguments' => [$id]])->first();
$home = $row['home'];   // Cassandra\UserTypeValue

$home->get('city');     // 'Paris'
$home->values();        // ['street' => ..., 'city' => ..., ...]

foreach ($home as $field => $value) {
    printf("%s = %s\n", $field, $value);
}
```

## Nesting

Composite types nest. Build the inner type first.

```php
// map<text, list<int>>
$type = Cassandra\Type::map(
    Cassandra\Type::text(),
    Cassandra\Type::collection(Cassandra\Type::int()),
);

$scores = $type->create();

$inner = Cassandra\Type::collection(Cassandra\Type::int())->create(10, 20, 30);
$scores->set('round1', $inner);
```

```php
// set<frozen<address>>
$addressType = $session->schema()->keyspace('shop')->userType('address');
$addresses   = Cassandra\Type::set($addressType)->create();

$addresses->add($addressType->create('city', 'Paris', 'country', 'FR'));
```

CQL requires `frozen<...>` for a user defined type inside a collection. The driver follows whatever
the schema declares.

## Reading collections

Collection columns come back as the matching class, never as a plain PHP array:

```php
$row = $session->execute('SELECT tags, attributes FROM posts')->first();

$row['tags'];        // Cassandra\Collection
$row['attributes'];  // Cassandra\Map

// Convert when a plain array is easier to work with:
$tags = $row['tags']->values();          // ['php', 'cql']
$attrs = [];
foreach ($row['attributes'] as $k => $v) {
    $attrs[(string) $k] = $v;
}
```

An empty collection in CQL is stored as `null`, so an empty list reads back as `null`, not as an
empty `Collection`.

```php
$tags = $row['tags']?->values() ?? [];
```

## Updating without a read

CQL updates collections in place. Do not read, change, and write back.

```php
// Append to a list.
$session->execute(
    'UPDATE posts SET tags = tags + ? WHERE id = ?',
    ['arguments' => [$newTags, $id]],
);

// Add to a set.
$session->execute(
    'UPDATE users SET roles = roles + ? WHERE id = ?',
    ['arguments' => [$newRoles, $id]],
);

// Set one map entry.
$session->execute(
    "UPDATE users SET attributes['logins'] = ? WHERE id = ?",
    ['arguments' => [new Cassandra\Bigint(13), $id]],
);
```

These updates are not idempotent. A retry can apply them twice. See
[retry policies](/guide/retry-policies#retries-and-idempotency).

## Size limits

A collection is read and written as a whole cell. Keep them small, in the low hundreds of elements.
For a growing list, model a clustering key instead:

```sql
-- Instead of events list<text> on the user row:
CREATE TABLE user_events (
    user_id uuid,
    event_id timeuuid,
    payload text,
    PRIMARY KEY (user_id, event_id)
) WITH CLUSTERING ORDER BY (event_id DESC);
```
