/*
  Copyright (c) DataStax, Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef DATASTAX_INTERNAL_CONNECTION_POOL_CONNECTOR_HPP
#define DATASTAX_INTERNAL_CONNECTION_POOL_CONNECTOR_HPP

#include "address.hpp"
#include "atomic.hpp"
#include "callback.hpp"
#include "connection_pool.hpp"
#include "connector.hpp"
#include "ref_counted.hpp"
#include "string.hpp"
#include "vector.hpp"

#include <uv.h>

namespace datastax { namespace internal { namespace core {

class ConnectionPoolManager;

/**
 * A connector for a connection pool. This handles the connection process for a
 * connection pool.
 */
class ConnectionPoolConnector : public RefCounted<ConnectionPoolConnector> {
public:
  typedef SharedRefPtr<ConnectionPoolConnector> Ptr;
  typedef Vector<Ptr> Vec;

  typedef internal::Callback<void, ConnectionPoolConnector*> Callback;

  /**
   * Constructor
   *
   * @param host The host to connect to.
   * @param protocol_version
   * @param callback A callback that is called when the connection is connected or
   * if an error occurred.
   */
  ConnectionPoolConnector(const Host::Ptr& host, ProtocolVersion protocol_version,
                          const Callback& callback);

  /**
   * Set the pool listener.
   *
   * @param listener A listener that handles pool events.
   * @return The connector to chain calls.
   */
  ConnectionPoolConnector* with_listener(ConnectionPoolListener* listener);

  /**
   * Set the keyspace to connect with. Calls "USE <keyspace>" after
   * the connection is connected and protocol handshake is completed.
   *
   * @param keyspace A keyspace to register after connection.
   * @return The connector to chain calls.
   */
  ConnectionPoolConnector* with_keyspace(const String& keyspace);

  /**
   * Set the metrics object to use to record metrics.
   *
   * @param metrics A metrics object.
   * @return The connector to chain calls.
   */
  ConnectionPoolConnector* with_metrics(Metrics* metrics);

  /**
   * Set the connection pool settings.
   *
   * @param The settings to use for connecting and maintaining the connection pools.
   * @return The connector to chain calls.
   */
  ConnectionPoolConnector* with_settings(const ConnectionPoolSettings& settings);

  /**
   * Set the calculator of local port (to be used by Connector when
   * port-based connection-to-shard routing is supported by the Host).
   *
   * @return The connector to chain calls.
   */
  ConnectionPoolConnector* with_shard_port_calculator(const ShardPortCalculator* shard_port_calculator);

  /**
   * Connect a pool. Opens `num_connections_per_host + 1` connections, the 1st being the
   * "scout connection", whose purpose is only to collect sharding info.
   */
  void connect_with_scout(uv_loop_t* loop);

  /**
   * Cancel the connection process.
   */
  void cancel();

  /**
   * Release the pool from the connector. If not released in the callback
   * the pool automatically be closed.
   *
   * @return The pool object for this connector. This returns a null object
   * if the pool is not connected or an error occurred.
   */
  ConnectionPool::Ptr release_pool();

public:
  const Address& address() const { return host_->address(); }

  Connector::ConnectionError error_code() const;
  String error_message() const;

  bool is_ok() const;
  bool is_critical_error() const;
  bool is_keyspace_error() const;

private:
  void connect();
  void on_connect(Connector* connector);
  void on_scout_connect(Connector* connector);

private:
  uv_loop_t* loop_;
  ConnectionPool::Ptr pool_;
  Callback callback_;
  bool is_canceled_;
  size_t remaining_;

  Connector::Vec pending_connections_;
  Connector::Ptr scout_connector_;
  Connection::Vec connections_;
  Connector::Ptr critical_error_connector_;

  Host::Ptr host_;
  const ProtocolVersion protocol_version_;
  ConnectionPoolSettings settings_;
  String keyspace_;
  ConnectionPoolListener* listener_;
  Metrics* metrics_;
  const ShardPortCalculator* shard_port_calculator_;
};

}}} // namespace datastax::internal::core

#endif
