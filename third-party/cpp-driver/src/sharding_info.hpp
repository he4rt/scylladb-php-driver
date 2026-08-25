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

#ifndef DATASTAX_INTERNAL_SHARDING_INFO_HPP
#define DATASTAX_INTERNAL_SHARDING_INFO_HPP

#include "decoder.hpp"
#include "optional.hpp"

namespace datastax { namespace internal { namespace core {

struct ConnectionShardingInfo;

class ShardingInfo final {
public:
  size_t get_shards_count() const;
  int32_t shard_id(int64_t token) const;
  CassOptional<int> shard_aware_port() const { return shard_aware_port_; }
  CassOptional<int> shard_aware_port_ssl() const { return shard_aware_port_ssl_; }

  static CassOptional<ConnectionShardingInfo> parse_sharding_info(const StringMultimap& params);

private:
  ShardingInfo(size_t shards_count, String partitioner, String sharding_algorithm, int sharding_ignore_MSB,
    CassOptional<int> shard_aware_port, CassOptional<int> shard_aware_port_ssl) noexcept;

  static const String SCYLLA_SHARD_PARAM_KEY;
  static const String SCYLLA_NR_SHARDS_PARAM_KEY;
  static const String SCYLLA_PARTITIONER;
  static const String SCYLLA_SHARDING_ALGORITHM;
  static const String SCYLLA_SHARDING_IGNORE_MSB;
  static const String SCYLLA_SHARD_AWARE_PORT;
  static const String SCYLLA_SHARD_AWARE_PORT_SSL;

  static CassOptional<String> parse_string(const StringMultimap& params, const String& key);
  static CassOptional<int> parse_int(const StringMultimap& params, const String& key);

  size_t shards_count_;
  String partitioner_;
  String sharding_algorithm_;
  int sharding_ignore_MSB_;
  CassOptional<int> shard_aware_port_;
  CassOptional<int> shard_aware_port_ssl_;
};

struct ConnectionShardingInfo final {
  int32_t shard_id;
  ShardingInfo sharding_info;
};

}}} // namespace datastax::internal::core

#endif
