# Results and paging

`execute()` returns a `Cassandra\Rows` object. It holds one page of the result set and knows how to
fetch the next one.

`Rows` implements `Iterator`, `Countable`, and `ArrayAccess`.

```php
$rows = $session->execute('SELECT id, email, login_count FROM users');

echo count($rows), " rows on this page\n";

foreach ($rows as $index => $row) {
    echo $index, ': ', $row['email'], "\n";
}

$firstRow = $rows[0];        // by offset
$firstRow = $rows->first();  // null when the page is empty
```

## Rows are associative arrays

Each row is a PHP array keyed by column name. Values arrive already decoded.

```php
$row = $session->execute('SELECT id, email, created_at, login_count FROM users')->first();

$row['id'];          // Cassandra\Uuid
$row['email'];       // string
$row['created_at'];  // Cassandra\Timestamp
$row['login_count']; // Cassandra\Bigint
```

A `null` column arrives as PHP `null`. The full mapping is in [data types](/guide/data-types).

::: tip Select the columns you need
`SELECT *` transfers and decodes every column. Naming the columns cuts network traffic and decode
cost, and it keeps the code working when someone adds a column.
:::

## Counting

```php
count($rows);   // rows on the current page, not in the whole result
```

`count()` never triggers a fetch. To count rows in the database, ask the database:

```php
$total = $session->execute('SELECT COUNT(*) FROM users')->first()['count'];
```

`COUNT(*)` without a partition key scans the whole table. Avoid it on large tables.

## Paging

The server returns at most `page_size` rows per response. The default is 5000. `Rows` holds one page.

### Automatic paging

```php
$rows = $session->execute($statement, ['page_size' => 1000]);

while (true) {
    foreach ($rows as $row) {
        process($row);
    }

    if ($rows->isLastPage()) {
        break;
    }

    $rows = $rows->nextPage();
}
```

`nextPage()` blocks while it fetches. It takes an optional timeout in seconds:

```php
$rows = $rows->nextPage(5.0);
```

::: warning Always check isLastPage() first
Call `isLastPage()` before `nextPage()`. Calling `nextPage()` past the end does not return a usable
result set.
:::

### Asynchronous paging

`nextPageAsync()` starts the fetch and returns a future at once, so the current page can be processed
while the next one travels.

```php
$rows = $session->execute($statement, ['page_size' => 1000]);

while (true) {
    $future = $rows->isLastPage() ? null : $rows->nextPageAsync();

    foreach ($rows as $row) {
        process($row);   // overlaps with the fetch
    }

    if ($future === null) {
        break;
    }

    $rows = $future->get();
}
```

This is the fastest way to walk a large result set. See [asynchronous queries](/guide/async).

### Stateless paging

Web applications cannot hold a `Rows` object between requests. Use the paging state token instead.
It is an opaque string that tells the server where to resume.

```php
// Request 1: the first page.
$rows  = $session->execute($statement, ['page_size' => 50]);
$token = $rows->pagingStateToken();   // null when there is no next page

// Send $token to the client, for example base64 encoded in a link.
$next = base64_encode($token);
```

```php
// Request 2: continue from the token.
$rows = $session->execute($statement, [
    'page_size'          => 50,
    'paging_state_token' => base64_decode($_GET['next']),
]);
```

::: danger Treat the token as opaque and untrusted
The token encodes a position in the result set. Do not build one, do not edit one, and do not accept
one from a client without care. Sign it, or store it server side and hand out a key.
:::

Paging tokens are tied to the exact query. Changing the CQL, the page size, or the bound values
invalidates the token.

## Choosing a page size

| Page size | Effect |
| --- | --- |
| Small, 100 to 500 | Low memory per page, more round trips. Good for user-facing lists. |
| Default, 5000 | A balanced choice for background work. |
| Large, above 10000 | Fewer round trips, but a large page can exceed the request timeout and raises memory use. |

Set it per query rather than globally when different queries need different values:

```php
$session->execute($listQuery,   ['page_size' => 50]);      // a page in a web view
$session->execute($exportQuery, ['page_size' => 5000]);    // a background export
```

## Iterating twice

`Rows` is a forward iterator with a rewind, so a second `foreach` over the same page works:

```php
foreach ($rows as $row) { /* first pass */ }
foreach ($rows as $row) { /* the same page again */ }
```

`nextPage()` returns a new `Rows` object rather than advancing the current one, so the page you hold
stays valid.

## Empty results

An empty result set is a valid `Rows` object, not `null` and not an exception.

```php
$rows = $session->execute($findById, ['arguments' => [$id]]);

if (count($rows) === 0) {
    throw new NotFoundException();
}

// Or:
$row = $rows->first();
if ($row === null) {
    throw new NotFoundException();
}
```

## Writes and schema changes

An `INSERT`, `UPDATE`, `DELETE`, or `CREATE TABLE` also returns a `Rows` object. It is empty, except
for a lightweight transaction, which returns the `[applied]` column. See
[lightweight transactions](/guide/queries#lightweight-transactions).
