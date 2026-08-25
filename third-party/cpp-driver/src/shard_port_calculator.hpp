/*
 * Copyright (C) 2020 ScyllaDB
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DATASTAX_INTERNAL_SHARD_PORT_CALCULATOR_HPP
#define DATASTAX_INTERNAL_SHARD_PORT_CALCULATOR_HPP

#include "optional.hpp"
#include <uv.h>
#include <map>
#include <stddef.h>

namespace datastax { namespace internal { namespace core {

/**
 * Server can be configured to route the client connections to specific shards,
 * based on their local port numbers. This class calculates outgoing port numbers
 * for such a case. It is intended to have 1 instance of this class per Cluster.
 */
class ShardPortCalculator final {
public:
  ShardPortCalculator(int local_port_range_lo, int local_port_range_hi);
  ShardPortCalculator(const ShardPortCalculator&) = delete;
  ShardPortCalculator& operator=(const ShardPortCalculator&) = delete;
  ~ShardPortCalculator();

  /**
   * Calculate a possible ephemeral port number, such that the targeted shard is
   * `port % shard_cnt` and `port` resides within the range [lo, hi). Thread-safe.
   */
  int calc_outgoing_port_num(int shard_cnt, int32_t desired_shard_id) const;

private:
  int local_port_range_lo_, local_port_range_hi_;

  /*
   * Map keeps track of used client-side ports. E.g. `port_states_[32000] == true`
   * means "port 32000 can be bound to local socket". It is NOT guaranteed that this
   * port will be available, but it gets us fewer unsuccessful connection attempts.
   */
  mutable std::map<int, bool> port_states_;
  mutable uv_mutex_t mutex_;
};

}}} // namespace datastax::internal::core

#endif // DATASTAX_INTERNAL_SHARD_PORT_CALCULATOR_HPP
