---
sidebar_position: 4
---

# Data Types

The PHP driver provides a set of classes to represent the various data types that are supported by Cassandra. The following table lists the Cassandra data types and their corresponding PHP classes.

Here's the list of Cassandra data types and their corresponding PHP classes:

| Scylla/Cassandra Type   | PHP Stub                  |
|-------------------------|---------------------------|
| Boolean                 | `bool` (native)           |
| Tinyint                 | `Cassandra\Tinyint`       |
| Smallint                | `Cassandra\Smallint`      |
| Int                     | `int` (native)            |
| BigInt                  | `Cassandra\Bigint`        |
| Float                   | `Cassandra\Float`         |
| Double                  | `float` (native)          |
| Ascii, Text, Varchar    | `string` (native)         |
| Counter                 | Not implemented           |
| Blob                    | `Cassandra\Blob`          |
| Inet                    | `Cassandra\Inet`          |
| Uuid                    | `Cassandra\Uuid`          |
| Timeuuid                | `Cassandra\Timeuuid`      |
| Date                    | `Cassandra\Date`          |
| Time                    | `Cassandra\Time`          |
| Timestamp               | `Cassandra\Timestamp`     |
| Duration                | `Cassandra\Duration`      |
| Decimal                 | `Cassandra\Decimal`       |
| Varint                  | `Cassandra\Varint`        |
| List                    | `Cassandra\Collection`    |
| Set                     | `Cassandra\Set`           |
| Map                     | `Cassandra\Map`           |
| Tuple                   | `Cassandra\Tuple`         |
| UDT (User defined type) | `Cassandra\UserTypeValue` |