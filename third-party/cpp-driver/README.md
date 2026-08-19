# C/C++ Driver for ScyllaDB

> [!IMPORTANT]
> :exclamation: **This driver is now superseded by [ScyllaDB CPP-RS Driver](https://github.com/scylladb/cpp-rs-driver) and no longer maintained.**
> Consider migration to the new driver for better performance and active support, especially of recent ScyllaDB features.

A modern, feature-rich and **shard-aware** C/C++ client library for
[ScyllaDB] using exclusively Cassandra's binary protocol and
Cassandra Query Language v3. Forked from Datastax `cpp-driver`.

## Getting the Driver

Currently we support CentOS 7, Ubuntu 18.04 and their relatives. On these
platforms [Installation] from packages is recommended. For other systems
(or hacking) we recommend to build the driver from sources.

Our releases use the versions of libuv, OpenSSL and zlib provided with the
distribution/EPEL.

## Features
* [Shard-Awareness]
* [Asynchronous API]
* [Simple], [Prepared], and [Batch] statements
* [Asynchronous I/O], [parallel execution], and request pipelining
* Connection pooling
* Automatic node discovery
* Automatic reconnection
* Configurable [load balancing]
* Works with any cluster size
* [Authentication]
* [SSL]
* [Latency-aware routing]
* [Performance metrics]
* [Tuples] and [UDTs]
* [Nested collections]
* [Retry policies]
* [Client-side timestamps]
* [Data types]
* [Idle connection heartbeats]
* Support for materialized view and secondary index metadata
* Support for clustering key order, `frozen<>` and Cassandra version metadata
* [Blacklist], [whitelist DC], and [blacklist DC] load balancing policies
* [Custom] authenticators
* [Reverse DNS] with SSL peer identity verification support
* Randomized contact points
* [Speculative execution]

## Compatibility

This driver works exclusively with the Cassandra Query Language v3 (CQL3) and
Cassandra's native protocol. The current version works with:

* Scylla and Scylla Enterprise
* Apache Cassandra® versions 2.1, 2.2 and 3.0+
* Architectures: 32-bit (x86) and 64-bit (x64)
* C++-11-conforming compilers

__Disclaimer__: cpp-driver does not support big-endian platforms.

## Documentation

* [Home]
* [API]
* [Getting Started]
* [Building]

## Training
The course [Using Scylla Drivers] in Scylla University explains how to use drivers in different languages to interact with a Scylla cluster. The lesson, [CPP Driver - Part 1], goes over a sample application that, using the CPP driver, interacts with a three-node Scylla cluster. It connects to a Scylla cluster, displays the contents of a table, inserts and deletes data, and shows the contents of the table after each action. [Scylla University] includes other training material and online courses which will help you become a Scylla NoSQL database expert.

## Getting Help

* Slack: http://slack.scylladb.com/
* `Issues` section of this repository

## Examples

The driver includes several examples in the [examples] directory.

## A Simple Example
```c
#include <cassandra.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  /* Setup and connect to cluster */
  CassFuture* connect_future = NULL;
  CassCluster* cluster = cass_cluster_new();
  CassSession* session = cass_session_new();
  char* hosts = "127.0.0.1";
  if (argc > 1) {
    hosts = argv[1];
  }

  /* Add contact points */
  cass_cluster_set_contact_points(cluster, hosts);

  /* Provide the cluster object as configuration to connect the session */
  connect_future = cass_session_connect(session, cluster);

  if (cass_future_error_code(connect_future) == CASS_OK) {
    CassFuture* close_future = NULL;

    /* Build statement and execute query */
    const char* query = "SELECT release_version FROM system.local";
    CassStatement* statement = cass_statement_new(query, 0);

    CassFuture* result_future = cass_session_execute(session, statement);

    if (cass_future_error_code(result_future) == CASS_OK) {
      /* Retrieve result set and get the first row */
      const CassResult* result = cass_future_get_result(result_future);
      const CassRow* row = cass_result_first_row(result);

      if (row) {
        const CassValue* value = cass_row_get_column_by_name(row, "release_version");

        const char* release_version;
        size_t release_version_length;
        cass_value_get_string(value, &release_version, &release_version_length);
        printf("release_version: '%.*s'\n", (int)release_version_length, release_version);
      }

      cass_result_free(result);
    } else {
      /* Handle error */
      const char* message;
      size_t message_length;
      cass_future_error_message(result_future, &message, &message_length);
      fprintf(stderr, "Unable to run query: '%.*s'\n", (int)message_length, message);
    }

    cass_statement_free(statement);
    cass_future_free(result_future);

    /* Close the session */
    close_future = cass_session_close(session);
    cass_future_wait(close_future);
    cass_future_free(close_future);
  } else {
    /* Handle error */
    const char* message;
    size_t message_length;
    cass_future_error_message(connect_future, &message, &message_length);
    fprintf(stderr, "Unable to connect: '%.*s'\n", (int)message_length, message);
  }

  cass_future_free(connect_future);
  cass_cluster_free(cluster);
  cass_session_free(session);

  return 0;
}
```

## Testing

This project includes a number of unit tests and an integration test suite. To run the integration tests against Scylla some prerequisites must be met:

* `scylla-ccm` cloned and installed system-wide
* `scylla-jmx` cloned alongside `scylla-ccm` and built
* `scylla-tools-java` cloned, built and symlinked from `[SCYLLA_ROOT]/resources/cassandra`

Building the integration tests:
```
mkdir build && cd build
cmake -DCASS_BUILD_INTEGRATION_TESTS=ON .. && make
```
Certain test cases require features that are unavailable in OSS Scylla, or fail for other non-critical reasons, and thus need to be disabled for now. Assuming that `scylla` is built in the release mode, the command line may look as below:
```
./cassandra-integration-tests --install-dir=[SCYLLA_ROOT] --version=3.0.8 --category=CASSANDRA --verbose=ccm --gtest_filter=-AuthenticationTests*:ConsistencyTwoNodeClusterTests.Integration_Cassandra_SimpleEachQuorum:ControlConnectionTests.Integration_Cassandra_TopologyChange:ControlConnectionTwoNodeClusterTests.Integration_Cassandra_Reconnection:CustomPayloadTests*:DbaasTests*:DcAwarePolicyTest.Integration_Cassandra_UsedHostsRemoteDc:ExecutionProfileTest.Integration_Cassandra_RequestTimeout:ExecutionProfileTest.Integration_Cassandra_SpeculativeExecutionPolicy:MetricsTests.Integration_Cassandra_SpeculativeExecutionRequests:MetricsTests.Integration_Cassandra_StatsConnections:PreparedTests.Integration_Cassandra_PreparedIDUnchangedDuringReprepare:ServerSideFailureTests.Integration_Cassandra_Warning:ServerSideFailureTests.Integration_Cassandra_ErrorFunctionFailure:ServerSideFailureTests.Integration_Cassandra_ErrorFunctionAlreadyExists:SessionTest.Integration_Cassandra_ExternalHostListener:SchemaMetadataTest*:SchemaNullStringApiArgsTest*:SpeculativeExecutionTests*:SslTests*:SslClientAuthenticationTests*
```

## License

&copy; DataStax, Inc.

Licensed under the Apache License, Version 2.0 (the “License”); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

Modified by ScyllaDB &copy; 2020

[ScyllaDB]: http://scylladb.com
[DataStax Enterprise]: http://www.datastax.com/products/datastax-enterprise
[Examples]: https://github.com/scylladb/cpp-driver/tree/master/examples
[GitHub]: https://github.com/scylladb/cpp-driver
[Home]: http://docs.datastax.com/en/developer/cpp-driver/latest
[API]: http://docs.datastax.com/en/developer/cpp-driver/latest/api
[Getting Started]: https://university.scylladb.com/courses/using-scylla-drivers/lessons/cpp-driver-part-1/
[Building]: http://docs.datastax.com/en/developer/cpp-driver/latest/topics/building
[Installation]: topics/
[Kerberos]: https://web.mit.edu/kerberos

[Shard-Awareness]:topics/scylla_specific/
[Asynchronous API]: http://datastax.github.io/cpp-driver/topics/#futures
[Simple]: http://datastax.github.io/cpp-driver/topics/#executing-queries
[Prepared]: http://datastax.github.io/cpp-driver/topics/basics/prepared_statements/
[Batch]: http://datastax.github.io/cpp-driver/topics/basics/batches/
[Asynchronous I/O]: http://datastax.github.io/cpp-driver/topics/#asynchronous-i-o
[parallel execution]: http://datastax.github.io/cpp-driver/topics/#thread-safety
[load balancing]: http://datastax.github.io/cpp-driver/topics/configuration/#load-balancing
[Authentication]: http://datastax.github.io/cpp-driver/topics/security/#authentication
[SSL]: http://datastax.github.io/cpp-driver/topics/security/ssl/
[Latency-aware routing]: http://datastax.github.io/cpp-driver/topics/configuration/#latency-aware-routing
[Performance metrics]: http://datastax.github.io/cpp-driver/topics/metrics/
[Tuples]: http://datastax.github.io/cpp-driver/topics/basics/tuples/
[UDTs]: http://datastax.github.io/cpp-driver/topics/basics/user_defined_types/
[Nested collections]: http://datastax.github.io/cpp-driver/topics/basics/binding_parameters/#nested-collections
[Data types]: http://datastax.github.io/cpp-driver/topics/basics/data_types/
[Retry policies]: http://datastax.github.io/cpp-driver/topics/configuration/retry_policies/
[Client-side timestamps]: http://datastax.github.io/cpp-driver/topics/basics/client_side_timestamps/
[Idle connection heartbeats]: http://datastax.github.io/cpp-driver/topics/configuration/#connection-heartbeats
[Blacklist]: http://datastax.github.io/cpp-driver/topics/configuration/#blacklist
[whitelist DC]: http://datastax.github.io/cpp-driver/topics/configuration/#datacenter
[blacklist DC]: http://datastax.github.io/cpp-driver/topics/configuration/#datacenter
[Custom]: http://datastax.github.io/cpp-driver/topics/security/#custom
[Reverse DNS]: http://datastax.github.io/cpp-driver/topics/security/ssl/#enabling-cassandra-identity-verification
[Speculative execution]: http://datastax.github.io/cpp-driver/topics/configuration/#speculative-execution
[Using Scylla Drivers]: https://university.scylladb.com/courses/using-scylla-drivers/
[CPP Driver - Part 1]: https://university.scylladb.com/courses/using-scylla-drivers/lessons/cpp-driver-part-1/
[Scylla University]: https://university.scylladb.com/
