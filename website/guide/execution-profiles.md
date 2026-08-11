# Execution profiles

An execution profile is a named bundle of execution settings. Register it on the cluster once, then
select it per query. Anything the profile does not set falls back to the cluster setting.

Use it when one application runs queries with different needs. A report scan wants a long timeout
and cheap consistency. A checkout write wants a short timeout and a local quorum. Without profiles
you set those per call and repeat yourself.

## Register and select

```php
use Cassandra\ExecutionProfile;

$cluster = Cassandra::cluster()
    ->withContactPoints('10.0.0.1')
    ->withExecutionProfile('analytics', (new ExecutionProfile())
        ->withConsistency(Cassandra::CONSISTENCY_LOCAL_ONE)
        ->withRequestTimeout(60.0))
    ->withExecutionProfile('oltp', (new ExecutionProfile())
        ->withConsistency(Cassandra::CONSISTENCY_LOCAL_QUORUM)
        ->withRequestTimeout(2.0))
    ->build();

$session = $cluster->connect('shop');

$rows = $session->execute($report, null, 'analytics');
$rows = $session->execute($order, null, 'oltp');
```

The profile is the third argument of `execute()` and `executeAsync()`. Pass `null`, or leave it out,
to use the cluster settings.

## Name a profile with an enum

A string name is easy to misspell, and a typo is only found at runtime. An enum fixes that.

```php
enum Profile: string
{
    case Analytics = 'analytics';
    case Oltp      = 'oltp';
}

$cluster = Cassandra::cluster()
    ->withExecutionProfile(Profile::Analytics, $analytics)
    ->withExecutionProfile(Profile::Oltp, $oltp)
    ->build();

$rows = $session->execute($report, null, Profile::Analytics);
```

Both the registration and the selector take a string or an enum case:

| Given | Name used |
| --- | --- |
| `'analytics'` | `analytics` |
| `enum P: string { case A = 'analytics'; }` | `analytics`, the case value |
| `enum P: int { case Fast = 1; }` | `Fast`, the case name |
| `enum P { case Reporting; }` | `Reporting`, the case name |

A string-backed enum contributes its value. Any other enum contributes its case name, because a
number is not a name.

::: warning A backed enum is not its case name
`Profile::Analytics` above registers `analytics`, not `Analytics`. Names are case-sensitive. Select a
profile with the same enum you registered it with and this never matters.
:::

## What a profile can set

| Method | Sets |
| --- | --- |
| `withConsistency(int $consistency)` | Consistency for the query |
| `withSerialConsistency(int $consistency)` | Serial consistency for a lightweight transaction |
| `withRequestTimeout(float $timeout)` | Seconds to wait for the server |
| `withRetryPolicy(RetryPolicy $policy)` | Retry policy |
| `withRoundRobinLoadBalancingPolicy()` | Plain round-robin |
| `withDatacenterAwareRoundRobinLoadBalancingPolicy(string $dc, int $hostsPerRemoteDc, bool $useRemoteForLocal)` | Prefer the local datacenter |
| `withTokenAwareRouting(bool $enabled = true)` | Route to a replica of the partition |
| `withTokenAwareRoutingShuffleReplicas(bool $enabled = true)` | Shuffle the replica list |
| `withLatencyAwareRouting(bool $enabled = true)` | Skip consistently slow nodes |
| `withLatencyAwareRoutingSettings(float $exclusionThreshold, float $scale, float $retryPeriod, float $updateRate, int $minMeasured)` | Tune the latency-aware policy |
| `withConstantSpeculativeExecutionPolicy(float $delay, int $max = 2)` | Re-send a slow request |
| `withNoSpeculativeExecutionPolicy()` | Turn speculative execution off |
| `withWhiteListHosts(string ...$hosts)` | Allow only these hosts |
| `withBlackListHosts(string ...$hosts)` | Never use these hosts |
| `withWhiteListDCs(string ...$dcs)` | Allow only these datacenters |
| `withBlackListDCs(string ...$dcs)` | Never use these datacenters |

Every method returns the same profile, so calls chain.

::: warning Speculative execution needs idempotent statements
A speculative attempt runs the statement more than once, so the driver only does it for statements
you mark idempotent. The policy has no effect until you call `setIdempotent()` on the statement or
pass the `idempotent` execution option. See [Idempotence](/guide/queries#idempotence).

Never put `withConstantSpeculativeExecutionPolicy()` on a profile you use for a counter update, a
lightweight transaction, or an append to a list.
:::

## A profile is copied when the cluster is built

`build()` copies the profile into the cluster. Changing the profile object afterwards does not reach
a cluster that already exists.

```php
$profile = (new ExecutionProfile())->withConsistency(Cassandra::CONSISTENCY_ONE);
$cluster = Cassandra::cluster()->withExecutionProfile('p', $profile)->build();

$profile->withConsistency(Cassandra::CONSISTENCY_ALL);   // $cluster still uses ONE
```

Build the profile fully, then build the cluster.

## Profiles and persistent clusters

Persistent clusters are cached per PHP worker, and the cache key covers the profiles. Two clusters
that differ only in a profile setting get their own cached cluster, so they never share settings by
accident.

The cost is one cache entry per distinct set of profiles. Build profiles from fixed values, not from
per-request data, or the cache grows with your traffic. See
[php.ini configuration](/guide/configuration) for the caps that bound it.

## Unknown profile names

Selecting a name that was never registered is an error from the server, not from the driver. Register
every profile on the cluster you connect with.
