# Bundled dependencies

## cpp-driver

The ScyllaDB C/C++ driver. Upstream stopped maintaining it, so the source lives
here and the extension builds it into `cassandra.so` by default.

`cpp-driver/UPSTREAM` records the repository and commit of the import. The tree
is edited in place, so `git log -- third-party/cpp-driver` shows every local
change, including fixes taken from `apache/cassandra-cpp-driver`.

Build against a driver installed on the system with
`-DPHP_SCYLLADB_USE_SYSTEM_DRIVER=ON`.

## sanitizers-cmake

A copy of arsenm/sanitizers-cmake, used by `-DENABLE_SANITIZERS=ON`.
