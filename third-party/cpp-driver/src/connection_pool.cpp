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

#include "connection_pool.hpp"

#include "config.hpp"
#include "connection_pool_manager.hpp"
#include "metrics.hpp"
#include "utils.hpp"

#include <algorithm>
#include <numeric>

using namespace datastax;
using namespace datastax::internal::core;

static inline bool least_busy_comp(const PooledConnection::Ptr& a, const PooledConnection::Ptr& b) {
  // Don't consider closed connections to be the least busy.
  if (a->is_closing()) { // "a" is closed so it can't be the least busy.
    return false;
  } else if (b->is_closing()) { // "a" is not close, but "b" is closed so "a" is less busy.
    return true;
  }
  // Both "a" and "b" are not closed so compare their inflight request counts.
  return a->inflight_request_count() < b->inflight_request_count();
}

ConnectionPoolSettings::ConnectionPoolSettings()
    : num_connections_per_host(CASS_DEFAULT_NUM_CONNECTIONS_PER_HOST)
    , reconnection_policy(new ConstantReconnectionPolicy()) {}

ConnectionPoolSettings::ConnectionPoolSettings(const Config& config)
    : connection_settings(config)
    , num_connections_per_host(config.core_connections_per_host())
    , reconnection_policy(config.reconnection_policy()) {}

class NopConnectionPoolListener : public ConnectionPoolListener {
public:
  virtual void on_pool_up(const Address& address) {}

  virtual void on_pool_down(const Address& address) {}

  virtual void on_pool_critical_error(const Address& address, Connector::ConnectionError code,
                                      const String& message) {}

  virtual void on_close(ConnectionPool* pool) {}
};

NopConnectionPoolListener nop_connection_pool_listener__;

ConnectionPool::ConnectionPool(const Connection::Vec& connections, ConnectionPoolListener* listener,
                               const String& keyspace, uv_loop_t* loop, const Host::Ptr& host,
                               ProtocolVersion protocol_version,
                               const ConnectionPoolSettings& settings, Metrics* metrics,
                               const ShardPortCalculator* shard_port_calculator)
    : listener_(listener ? listener : &nop_connection_pool_listener__)
    , keyspace_(keyspace)
    , loop_(loop)
    , host_(host)
    , protocol_version_(protocol_version)
    , settings_(settings)
    , metrics_(metrics)
    , shard_port_calculator_(shard_port_calculator)
    , close_state_(CLOSE_STATE_OPEN)
    , notify_state_(NOTIFY_STATE_NEW) {
  inc_ref(); // Reference for the lifetime of the pooled connections
  set_pointer_keys(reconnection_schedules_);
  set_pointer_keys(to_flush_);

  const auto& si = host_->sharding_info();
  if (si) {
    const auto hosts_shard_cnt = si->get_shards_count();
    connections_by_shard_.resize(hosts_shard_cnt);
    num_connections_per_shard_ = settings_.num_connections_per_host / hosts_shard_cnt
        + (settings_.num_connections_per_host % hosts_shard_cnt ? 1u : 0u);
  } else {
    connections_by_shard_.resize(1);
    num_connections_per_shard_ = settings_.num_connections_per_host;
  }

  for (Connection::Vec::const_iterator it = connections.begin(), end = connections.end(); it != end;
       ++it) {
    const Connection::Ptr& connection(*it);
    if (!connection->is_closing()) {
      if (connections_by_shard_[connection->shard_id()].size() < num_connections_per_shard_) {
        add_connection(PooledConnection::Ptr(new PooledConnection(this, connection)));
      } else {
        host_->add_unpooled_connection(connection);
      }
    }
  }

  grab_unpooled_connections_from_host();

  notify_up_or_down();

  // We had non-critical errors or some connections closed
  for (size_t shard_num = 0u; shard_num < connections_by_shard_.size(); ++shard_num) {
    const int needed = num_connections_per_shard_ - connections_by_shard_[shard_num].size();
    for (int32_t i = 0; i < needed; ++i) {
      if (si && (si->shard_aware_port() || si->shard_aware_port_ssl())) {
        // Reconnect with port-based shard awareness
        schedule_reconnect(nullptr, shard_num);
      } else {
        // Traditional or no shard awareness
        schedule_reconnect();
      }
    }
  }
}

PooledConnection::Ptr ConnectionPool::find_least_busy(int64_t token) const {
  if (token == CASS_INT64_MIN || !host_->sharding_info()) {
    // We got a placeholder token, or a sensible token that is useless without the sharding info.
    // In both cases we return the least busy connection of the *entire pool* (or NULL).
    PooledConnection::Ptr least_busy; // NULL by default
    for (const auto& shard_pool : connections_by_shard_) {
      for (const auto& conn : shard_pool) {
        if (!conn->is_closing()) {
          least_busy = least_busy ? std::min(least_busy, conn, least_busy_comp) : conn;
        }
      }
    }
    return least_busy;
  }

  // Otherwise, find the least busy connection pointing to the right shard (if possible)
  const auto& shard_pool = connections_by_shard_[host_->sharding_info()->shard_id(token)];
  PooledConnection::Vec::const_iterator it =
      std::min_element(shard_pool.begin(), shard_pool.end(), least_busy_comp);
  if (it == shard_pool.end() || (*it)->is_closing()) {
    return find_least_busy(CASS_INT64_MIN);
  }
  return *it;
}

bool ConnectionPool::has_connections() const {
  return std::any_of(connections_by_shard_.begin(), connections_by_shard_.end(),
      [] (const PooledConnection::Vec& v) { return !v.empty(); });
}

void ConnectionPool::flush() {
  for (DenseHashSet<PooledConnection*>::const_iterator it = to_flush_.begin(),
                                                       end = to_flush_.end();
       it != end; ++it) {
    (*it)->flush();
  }
  to_flush_.clear();
}

void ConnectionPool::close() { internal_close(); }

void ConnectionPool::attempt_immediate_connect() {
  for (DelayedConnector::Vec::iterator it = pending_connections_.begin(),
                                       end = pending_connections_.end();
       it != end; ++it) {
    (*it)->attempt_immediate_connect();
  }
}

void ConnectionPool::set_listener(ConnectionPoolListener* listener) {
  listener_ = listener ? listener : &nop_connection_pool_listener__;
}

void ConnectionPool::set_keyspace(const String& keyspace) { keyspace_ = keyspace; }

void ConnectionPool::requires_flush(PooledConnection* connection, ConnectionPool::Protected) {
  if (to_flush_.empty()) {
    listener_->on_requires_flush(this);
  }
  to_flush_.insert(connection);
}

void ConnectionPool::close_connection(PooledConnection* connection, Protected) {
  if (metrics_) {
    metrics_->total_connections.dec();
  }
  auto& shard_pool = connections_by_shard_[connection->shard_id()];
  shard_pool.erase(std::remove(shard_pool.begin(), shard_pool.end(), connection),
                     shard_pool.end());
  to_flush_.erase(connection);

  if (close_state_ != CLOSE_STATE_OPEN) {
    maybe_closed();
    return;
  }

  // When there are no more connections available then notify that the host
  // is down.
  notify_up_or_down();
  // Try to reconnect to the same shard.
  schedule_reconnect(nullptr, connection->shard_id());
}

void ConnectionPool::add_connection(const PooledConnection::Ptr& connection) {
  if (metrics_) {
    metrics_->total_connections.inc();
  }
  const size_t new_connections_shard = connection->shard_id();
  LOG_INFO("add_connection: to host %s to shard %ld", host_->address_string().c_str(), new_connections_shard);
  connections_by_shard_[new_connections_shard].push_back(connection);
}

void ConnectionPool::notify_up_or_down() {
  if ((notify_state_ == NOTIFY_STATE_NEW || notify_state_ == NOTIFY_STATE_UP) && !has_connections()) {
    notify_state_ = NOTIFY_STATE_DOWN;
    listener_->on_pool_down(host_->address());
  } else if ((notify_state_ == NOTIFY_STATE_NEW || notify_state_ == NOTIFY_STATE_DOWN) && has_connections()) {
    notify_state_ = NOTIFY_STATE_UP;
    listener_->on_pool_up(host_->address());
  }
}

void ConnectionPool::notify_critical_error(Connector::ConnectionError code, const String& message) {
  if (notify_state_ != NOTIFY_STATE_CRITICAL) {
    notify_state_ = NOTIFY_STATE_CRITICAL;
    listener_->on_pool_critical_error(host_->address(), code, message);
  }
}

void ConnectionPool::schedule_reconnect(ReconnectionSchedule* schedule, CassOptional<int32_t> desired_shard_num) {
  DelayedConnector::Ptr connector(new DelayedConnector(
      host_, protocol_version_, bind_callback(&ConnectionPool::on_reconnect, this)));

  if (!schedule) {
    schedule = settings_.reconnection_policy->new_reconnection_schedule();
  }
  reconnection_schedules_[connector.get()] = schedule;

  uint64_t delay_ms = schedule->next_delay_ms();

  if (desired_shard_num) {
    const auto& si = host_->sharding_info();
    if (si && (si->shard_aware_port() || si->shard_aware_port_ssl())) {
      connector->set_desired_shard_num(*desired_shard_num);
    }
  }

  LOG_INFO("Scheduling %s reconnect for host %s in %llums on connection pool (%p) ",
           settings_.reconnection_policy->name(), host_->address().to_string().c_str(),
           static_cast<unsigned long long>(delay_ms), static_cast<void*>(this));

  pending_connections_.push_back(connector);
  connector->with_keyspace(keyspace())
      ->with_metrics(metrics_)
      ->with_settings(settings_.connection_settings)
      ->with_shard_port_calculator(shard_port_calculator_)
      ->delayed_connect(loop_, delay_ms);
}

void ConnectionPool::internal_close() {
  if (close_state_ == CLOSE_STATE_OPEN) {
    close_state_ = CLOSE_STATE_CLOSING;

    // Make copies of connection/connector data structures to prevent iterator
    // invalidation.

    auto connections_per_shards = connections_by_shard_;
    std::for_each(connections_per_shards.begin(), connections_per_shards.end(), [] (PooledConnection::Vec& v) {
      for (auto& c : v) {
        c->close();
      }
    });

    DelayedConnector::Vec pending_connections(pending_connections_);
    for (DelayedConnector::Vec::iterator it = pending_connections.begin(),
                                         end = pending_connections.end();
         it != end; ++it) {
      (*it)->cancel();
    }

    host_->close_unpooled_connections(loop());

    close_state_ = CLOSE_STATE_WAITING_FOR_CONNECTIONS;
    maybe_closed();
  }
}

void ConnectionPool::maybe_closed() {
  // Remove the pool once all current connections and pending connections
  // are terminated.
  if (close_state_ == CLOSE_STATE_WAITING_FOR_CONNECTIONS && !has_connections() && pending_connections_.empty()) {
    close_state_ = CLOSE_STATE_CLOSED;
    // Only mark DOWN if it's UP otherwise we might get multiple DOWN events
    // when connecting the pool.
    if (notify_state_ == NOTIFY_STATE_UP) {
      listener_->on_pool_down(host_->address());
    }
    listener_->on_close(this);
    dec_ref();
  }
}

bool ConnectionPool::grab_unpooled_connections_from_host() {
  bool is_pool_saturated = true;

  for (int shard_id = 0; shard_id < (int)connections_by_shard_.size(); ++shard_id) {
    is_pool_saturated &= grab_unpooled_connections_from_host(shard_id);
  }

  return is_pool_saturated;
}

bool ConnectionPool::grab_unpooled_connections_from_host(int shard_id) {
  const auto num_needed_connections = num_connections_per_shard_ - connections_by_shard_[shard_id].size();
  if (num_needed_connections == 0) {
    return true;
  }

  auto unpooled_connections = host_->get_unpooled_connections(shard_id, num_needed_connections);

  for (const auto& connection : unpooled_connections) {
    add_connection(PooledConnection::Ptr(new PooledConnection(this, connection->import_connection(loop_))));
  }

  return connections_by_shard_[shard_id].size() == num_connections_per_shard_;
}

void ConnectionPool::on_reconnect(DelayedConnector* connector) {
  pending_connections_.erase(
      std::remove(pending_connections_.begin(), pending_connections_.end(), connector),
      pending_connections_.end());

  ReconnectionSchedules::iterator it = reconnection_schedules_.find(connector);
  assert(it != reconnection_schedules_.end() &&
         "No reconnection schedule associated with connector");

  ScopedPtr<ReconnectionSchedule> schedule(it->second);
  reconnection_schedules_.erase(it);

  if (close_state_ != CLOSE_STATE_OPEN) {
    maybe_closed();
    return;
  }

  if (connector->is_ok()) {
    auto connection = connector->release_connection();
    const size_t new_connections_shard = connection->shard_id();
    if (connections_by_shard_[new_connections_shard].size() < num_connections_per_shard_) {
      add_connection(PooledConnection::Ptr(new PooledConnection(this, connection)));
      notify_up_or_down();
    } else {
      LOG_INFO("Reconnection to host %s connected us to shard %ld",
               address().to_string().c_str(), new_connections_shard);
      host_->add_unpooled_connection(std::move(connection));
      if (!grab_unpooled_connections_from_host() && reconnection_schedules_.empty()) {
        /* The check `reconnection_schedules_.empty()` is not obligatory. It is a heuristic to avoid opening too many connections:
        - we have pending reconnections in progress => maybe they'll give us right connections (with the help of marketplace);
        - we need more connections but no reconnects are in progress => OK, let's open just one.
        Experiments confirm that it helps - reduces unpooled connections by ~2x. */
        schedule_reconnect(schedule.release(), connector->desired_shard_num());
      }
    }
  } else if (!connector->is_canceled()) {
    if (connector->is_critical_error()) {
      LOG_ERROR("Closing established connection pool to host %s because of the following error: %s",
                address().to_string().c_str(), connector->error_message().c_str());
      notify_critical_error(connector->error_code(), connector->error_message());
      internal_close();
    } else {
      LOG_WARN(
          "Connection pool was unable to reconnect to host %s because of the following error: %s",
          address().to_string().c_str(), connector->error_message().c_str());
      if (!grab_unpooled_connections_from_host()) {
        schedule_reconnect(schedule.release(), connector->desired_shard_num());
      }
    }
  }
}
