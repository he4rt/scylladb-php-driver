# Load balancing and routing

Every request must go to a node. Three independent mechanisms decide which one:

1. **The load balancing policy** builds an ordered list of candidate nodes.
2. **Token-aware routing** reorders that list to put data owners first.
3. **Host and datacenter filters** remove nodes from the list before anything else runs.

The settings combine. A common production setup uses a datacenter-aware policy plus token-aware
routing.

## Load balancing policies

### Round robin (default)

```php
->withRoundRobinLoadBalancingPolicy()
```

The driver cycles through every node in the cluster, in every datacenter. This is the default and it
is correct only when the application and the whole cluster sit in one datacenter.

### Datacenter-aware round robin

```php
->withDatacenterAwareRoundRobinLoadBalancingPolicy(
    localDatacenter: 'eu-west-1',
    hostPerRemoteDatacenter: 0,
    useRemoteDatacenterForLocalConsistencies: false,
)
```

Nodes in the local datacenter are used first. Use this policy whenever the cluster spans more than
one datacenter or region.

| Parameter | Meaning |
| --- | --- |
| `localDatacenter` | The datacenter name as the cluster reports it, for example `eu-west-1`. It must match exactly. |
| `hostPerRemoteDatacenter` | How many nodes per remote datacenter the driver may fall back to when the local datacenter has no node available. `0` disables remote fallback. |
| `useRemoteDatacenterForLocalConsistencies` | Whether a remote node may serve a request that asks for `LOCAL_ONE` or `LOCAL_QUORUM`. Keep this `false`. |

All three parameters are required. There is no default.

::: warning Two ways to lose datacenter isolation
`hostPerRemoteDatacenter` above `0` sends cross-region traffic during a local outage, which raises
latency but keeps the application alive. `useRemoteDatacenterForLocalConsistencies: true` breaks the
guarantee that `LOCAL_QUORUM` means "local". Set it to `true` only after a deliberate decision.
:::

Find the datacenter name from a node:

```bash
nodetool status | head -3
```

Or from CQL:

```sql
SELECT data_center, rack FROM system.local;
```

## Token-aware routing

```php
->withTokenAwareRouting(true)   // on by default
```

The driver hashes the partition key of the statement, computes the token, and sends the request
straight to a replica that owns it. Without it, the coordinator has to forward the request, which
adds a network hop and load on a node that holds none of the data.

Two conditions must hold for it to work:

- **The statement must be prepared.** A prepared statement carries partition key metadata. A
  `SimpleStatement` does not, so the driver cannot compute a token for it.
- **Schema metadata must be on.** It is on by default. `withSchemaMetadata(false)` turns off token
  awareness as a side effect.

This is the strongest single reason to prepare statements. See
[performance](/guide/performance).

## Latency-aware routing

```php
->withLatencyAwareRouting(true)   // on by default
```

The driver measures response latency per node and moves consistently slow nodes to the back of the
candidate list. It helps against a node that is degraded rather than down, which normal failure
detection does not catch.

Turn it off when node latency is naturally uneven and the penalty causes traffic to bunch up on a
few nodes.

## Host and datacenter filters

Four filters remove nodes from consideration. Each takes a variadic list.

```php
->withWhiteListHosts('10.0.0.1', '10.0.0.2')   // use only these nodes
->withBlackListHosts('10.0.0.9')               // never use these nodes
->withWhiteListDCs('eu-west-1')                // use only these datacenters
->withBlackListDCs('us-east-1')                // never use these datacenters
```

Use them for:

- Pinning a batch job to a small set of nodes so it does not disturb the serving path.
- Taking a node out of rotation from the client side during maintenance.
- Keeping an application inside one datacenter without changing the load balancing policy.

::: tip Prefer the datacenter-aware policy
A datacenter whitelist and the datacenter-aware policy solve overlapping problems. The policy is the
better tool, because it keeps a controlled fallback path. Reach for the filters when you need a hard
boundary.
:::

A whitelist that excludes every reachable node makes `connect()` fail. There is no fallback.

## Consistency and routing together

Routing decides which node receives the request. Consistency decides how many replicas must
acknowledge it. They interact:

| Consistency | Behavior with a datacenter-aware policy |
| --- | --- |
| `CONSISTENCY_LOCAL_ONE` | One replica in the local datacenter answers. |
| `CONSISTENCY_LOCAL_QUORUM` | A quorum inside the local datacenter answers. The usual production choice. |
| `CONSISTENCY_QUORUM` | A quorum across all datacenters answers, so a remote round trip is likely. |
| `CONSISTENCY_EACH_QUORUM` | A quorum in every datacenter answers. Writes only. |
| `CONSISTENCY_ALL` | Every replica answers. One node down means the request fails. |

Set the cluster default once and override it per statement where needed:

```php
$cluster = Cassandra::cluster()
    ->withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_QUORUM)
    ->build();

// Override for one query that tolerates staleness.
$rows = $session->execute($stmt, ['consistency' => Cassandra::CONSISTENCY_LOCAL_ONE]);
```

The cluster default is `Cassandra::CONSISTENCY_LOCAL_QUORUM`. It reads and writes a majority of the
replicas in the local datacenter, so a read sees the last write. Drop to `LOCAL_ONE` for a query
that values latency over freshness, such as a cache fill or an analytics scan.

Do not set plain `QUORUM` to get stronger guarantees. `QUORUM` counts replicas across every
datacenter, so it adds cross-datacenter latency to every query and fails when a remote datacenter is
unreachable. `LOCAL_QUORUM` is the level that gives strong consistency at local latency.

The full list is in the [constants reference](/reference/constants).

## A production configuration

```php
$cluster = Cassandra::cluster()
    ->withContactPoints('10.1.0.11', '10.1.0.12', '10.1.0.13')
    ->withDatacenterAwareRoundRobinLoadBalancingPolicy('eu-west-1', 0, false)
    ->withTokenAwareRouting(true)
    ->withLatencyAwareRouting(true)
    ->withDefaultConsistency(Cassandra::CONSISTENCY_LOCAL_QUORUM)
    ->build();
```

Contact points name local nodes. The policy keeps traffic local. Token awareness removes the extra
hop. Consistency stays inside the datacenter.
