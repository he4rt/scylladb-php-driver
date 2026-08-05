# Schema metadata

The driver keeps a live copy of the cluster schema. `Session::schema()` returns it.

```php
$schema = $session->schema();
```

Metadata is fetched at connect time and updated when the schema changes. Reading it costs no network
call.

::: tip Metadata requires withSchemaMetadata(true)
It is on by default. Turning it off also turns off
[token-aware routing](/guide/load-balancing#token-aware-routing).
:::

## Keyspaces

```php
$schema->keyspaces();          // array<string, Cassandra\Keyspace>
$schema->keyspace('shop');     // Cassandra\Keyspace, or false when absent

$keyspace = $schema->keyspace('shop');

$keyspace->name();                    // 'shop'
$keyspace->replicationClassName();    // 'org.apache.cassandra.locator.NetworkTopologyStrategy'
$keyspace->replicationOptions();      // ['eu-west-1' => '3', ...]
$keyspace->hasDurableWrites();        // true
```

`keyspace()` returns `false` for a name that does not exist. Check before you use it:

```php
$keyspace = $schema->keyspace($name);

if ($keyspace === false) {
    throw new RuntimeException("Keyspace $name does not exist");
}
```

## Tables

```php
$keyspace->tables();            // array<string, Cassandra\Table>
$keyspace->table('users');      // Cassandra\Table, or false

$table = $keyspace->table('users');

$table->name();
$table->comment();
$table->options();              // every option, as an array
$table->option('bloom_filter_fp_chance');
```

Named accessors exist for the common options. Each returns `false` when the option is not set:

```php
$table->defaultTTL();
$table->gcGraceSeconds();
$table->caching();
$table->bloomFilterFPChance();
$table->compactionStrategyClassName();
$table->compactionStrategyOptions();
$table->compressionParameters();
$table->speculativeRetry();
$table->memtableFlushPeriodMs();
$table->minIndexInterval();
$table->maxIndexInterval();
```

### Keys

```php
$table->partitionKey();      // array<int, Column>
$table->clusteringKey();     // array<int, Column>
$table->primaryKey();        // partition key plus clustering key
$table->clusteringOrder();   // array<int, string>, for example ['DESC']
```

```php
$partition = array_map(fn ($c) => $c->name(), $table->partitionKey());

echo 'PRIMARY KEY ((', implode(', ', $partition), '))', PHP_EOL;
```

## Columns

```php
$table->columns();            // array<string, Cassandra\Column>
$table->column('email');      // Cassandra\Column, or false

$column = $table->column('email');

$column->name();          // 'email'
$column->type();          // Cassandra\Type, or null
$column->isStatic();      // bool
$column->isFrozen();      // bool
$column->isReversed();    // bool, true for a DESC clustering column
$column->indexName();     // string, or null
$column->indexOptions();  // string, or null
```

Print a table definition:

```php
foreach ($table->columns() as $name => $column) {
    printf("%-24s %s%s\n", $name, $column->type(), $column->isStatic() ? ' static' : '');
}
```

## User defined types

This is the most useful part of the metadata for application code. It removes the need to declare
field types by hand.

```php
$type = $keyspace->userType('address');    // Cassandra\Type\UserType, or null

$type->name();        // 'address'
$type->keyspace();    // 'shop'
$type->types();       // array<string, Cassandra\Type>

$address = $type->create(
    'street',  '1 Rue de Rivoli',
    'city',    'Paris',
    'country', 'FR',
);
```

```php
$keyspace->userTypes();   // array<string, Cassandra\Type\UserType>
```

See [collections and user defined types](/guide/collections#user-defined-types).

## Materialized views

```php
$keyspace->materializedViews();                 // array<string, MaterializedView>
$keyspace->materializedView('users_by_email');  // MaterializedView, or false

$view = $keyspace->materializedView('users_by_email');

$view->baseTable();   // Cassandra\Table, or null
$view->columns();     // a view is a Table, so every Table method works
```

`Cassandra\MaterializedView` extends `Cassandra\Table`.

## Functions and aggregates

```php
$keyspace->functions();                             // array<string, Cassandra\Function_>
$keyspace->function('to_upper', Cassandra::TYPE_TEXT);

$keyspace->aggregates();
$keyspace->aggregate('sum_all', Cassandra::TYPE_INT);
```

Both take the name plus the argument types, because CQL allows overloading. The PHP class is named
`Cassandra\Function_` because `function` is a reserved word.

## Practical uses

### Check that a table exists before you use it

```php
function requireTable(Cassandra\Session $session, string $ks, string $table): Cassandra\Table
{
    $keyspace = $session->schema()->keyspace($ks);

    if ($keyspace === false) {
        throw new RuntimeException("Missing keyspace $ks");
    }

    $found = $keyspace->table($table);

    if ($found === false) {
        throw new RuntimeException("Missing table $ks.$table");
    }

    return $found;
}
```

Run this at process start. A missing table then fails at boot, not on the first customer request.

### Verify a migration

```php
$columns = array_keys($session->schema()->keyspace('shop')->table('users')->columns());

$expected = ['id', 'email', 'created_at', 'login_count'];
$missing  = array_diff($expected, $columns);

if ($missing !== []) {
    throw new RuntimeException('Missing columns: ' . implode(', ', $missing));
}
```

### Generate a schema report

```php
foreach ($session->schema()->keyspaces() as $name => $keyspace) {
    if (str_starts_with($name, 'system')) {
        continue;
    }

    printf("%s (%s)\n", $name, $keyspace->replicationClassName());

    foreach ($keyspace->tables() as $tableName => $table) {
        printf("  %-30s %d columns\n", $tableName, count($table->columns()));
    }
}
```

## Freshness

Metadata updates when the cluster pushes a schema change event. A change made through a different
session, or by an operator, reaches the driver shortly after it applies. Code that creates a table
and immediately reads its metadata may see the old view. Re-read `schema()` after a short wait when
that matters.
