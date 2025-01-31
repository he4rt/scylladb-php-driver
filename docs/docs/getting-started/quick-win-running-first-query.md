---
sidebar_position: 2
---

# Running the First Query

Now that you have the driver installed, let's run a simple query to make sure everything is working as expected.

But before that, we need at least a ScyllaDB node running on your local machine. If you don't have one, you can follow
the [ScyllaDB Downloads page](https://www.scylladb.com/download/#core) learn more.

## Running a ScyllaDB Node Instance

For now, we can use Docker to run a ScyllaDB node. You can run the following command to start a ScyllaDB node:

```bash
docker run --name node1 --network ws-scylla -p "9042:9042" -d scylladb/scylla:6.2.2 \
  --overprovisioned 1 \
  --smp 1
```
To check the status of your node, copy and run this command in the [terminal](tab-0) tab:

```bash
docker exec -it node1 nodetool status
```

You should see something that looks like this:

```text
Datacenter: datacenter1
=======================
Status=Up/Down
|/ State=Normal/Leaving/Joining/Moving
--  Address     Load       Tokens       Owns    Host ID                               Rack
UN  172.17.0.2  204 KB     256          ?       b26ab7f8-17ee-4e51-b28a-523697d16a60  rack1
```

## Running the First Query

Now that you have a ScyllaDB node running, let's run a simple query to make sure everything is working as expected.

Create a new file called `first-query.php` and add the following code:

```php
$cluster   = Cassandra::cluster()->build();
$keyspace  = 'system';
$session   = $cluster->connect($keyspace);        // create session, optionally scoped to a keyspace
$statement = new \Cassandra\SimpleStatement(       // also supports prepared and batch statements
    'SELECT * from system.local'
);
$querySent = $session->execute($statement);

foreach ($querySent as $row) {                       // results and rows implement Iterator, Countable and ArrayAccess
    echo "The cluster is in datacenter: " . $row['data_center'] . "\n";
}
```

Now, run the following command in your terminal:

```bash
php first-query.php
```

The output should be:

```text
===== Using optimized driver!!! =====
The cluster is in datacenter: datacenter1
```