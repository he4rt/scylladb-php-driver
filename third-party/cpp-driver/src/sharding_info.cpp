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

#include "sharding_info.hpp"

#include <limits>

namespace datastax { namespace internal { namespace core {

const String ShardingInfo::SCYLLA_SHARD_PARAM_KEY = "SCYLLA_SHARD";
const String ShardingInfo::SCYLLA_NR_SHARDS_PARAM_KEY = "SCYLLA_NR_SHARDS";
const String ShardingInfo::SCYLLA_PARTITIONER = "SCYLLA_PARTITIONER";
const String ShardingInfo::SCYLLA_SHARDING_ALGORITHM = "SCYLLA_SHARDING_ALGORITHM";
const String ShardingInfo::SCYLLA_SHARDING_IGNORE_MSB = "SCYLLA_SHARDING_IGNORE_MSB";
const String ShardingInfo::SCYLLA_SHARD_AWARE_PORT = "SCYLLA_SHARD_AWARE_PORT";
const String ShardingInfo::SCYLLA_SHARD_AWARE_PORT_SSL = "SCYLLA_SHARD_AWARE_PORT_SSL";

ShardingInfo::ShardingInfo(size_t shards_count, String partitioner, String sharding_algorithm, int sharding_ignore_MSB,
    CassOptional<int> shard_aware_port, CassOptional<int> shard_aware_port_ssl) noexcept
  : shards_count_(shards_count)
  , partitioner_(std::move(partitioner))
  , sharding_algorithm_(std::move(sharding_algorithm))
  , sharding_ignore_MSB_(sharding_ignore_MSB)
  , shard_aware_port_(std::move(shard_aware_port))
  , shard_aware_port_ssl_(std::move(shard_aware_port_ssl)) {}

size_t ShardingInfo::get_shards_count() const {
  return shards_count_;
}

int32_t ShardingInfo::shard_id(int64_t token) const {
  token += std::numeric_limits<int64_t>::min();
  token <<= sharding_ignore_MSB_;
  const int64_t tokLo = token & 0xffffffffL;
  const int64_t tokHi = (token >> 32) & 0xffffffffL;
  const int64_t mul1 = tokLo * shards_count_;
  const int64_t mul2 = tokHi * shards_count_; // logically shifted 32 bits
  const int64_t sum = (mul1 >> 32) + mul2;
  return (int32_t) (sum >> 32);
}

CassOptional<ConnectionShardingInfo> ShardingInfo::parse_sharding_info(const StringMultimap& params) {
  const auto shard_id = parse_int(params, SCYLLA_SHARD_PARAM_KEY);
  const auto shards_count = parse_int(params, SCYLLA_NR_SHARDS_PARAM_KEY);
  const auto partitioner = parse_string(params, SCYLLA_PARTITIONER);
  const auto sharding_algorithm = parse_string(params, SCYLLA_SHARDING_ALGORITHM);
  const auto sharding_ignore_MSB = parse_int(params, SCYLLA_SHARDING_IGNORE_MSB);
  const auto shard_aware_port = parse_int(params, SCYLLA_SHARD_AWARE_PORT);
  const auto shard_aware_port_ssl = parse_int(params, SCYLLA_SHARD_AWARE_PORT_SSL);

  if (!shard_id || !shards_count || !partitioner || !sharding_algorithm || !sharding_ignore_MSB
      || *partitioner != "org.apache.cassandra.dht.Murmur3Partitioner"
      || *sharding_algorithm != "biased-token-round-robin") {
    return CassNullopt;
  }
  return ConnectionShardingInfo{*shard_id,
      ShardingInfo{(size_t)(*shards_count), *partitioner, *sharding_algorithm, *sharding_ignore_MSB,
          shard_aware_port, shard_aware_port_ssl}};
}

CassOptional<String> ShardingInfo::parse_string(const StringMultimap& params, const String& key) {
  if (!params.count(key) || params.at(key).size() != 1u) {
      return CassNullopt;
  }
  return params.at(key)[0];
}

CassOptional<int> ShardingInfo::parse_int(const StringMultimap& params, const String& key) {
  const auto val = parse_string(params, key);
  if (!val) {
    return CassNullopt;
  }
  return std::atoi(val->c_str());
}

}}} // namespace datastax::internal::core
