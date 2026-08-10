/**
 * Copyright 2015-2017 DataStax, Inc.
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

#include <zend_smart_str.h>

#include <cassandra.h>

#include <php_scylladb.h>
#include <Zend/zend_enum.h>
#include <php_scylladb_cache_key.h>
#include <SSLOptions/SSLOptions.h>
#include <php_scylladb_globals.h>
#include <php_scylladb_types.h>
#include <php_scylladb_consistency.h>

#include "Cluster.h"
#include "zend_portability.h"

#include "Builder_arginfo.h"

extern zend_object_handlers php_scylladb_cluster_builder_handlers;

static zend_always_inline zend_string *php_scylladb_build_hosts_str(const zval *args,
                                                                  const size_t argc)
{
    smart_str hosts = (smart_str){nullptr, 0};

    for (size_t i = 0; i < argc; i++)
    {
        const zval *host = &args[i];

        if (Z_TYPE_P(host) != IS_STRING)
        {
            smart_str_free(&hosts);
            return nullptr;
        }

        if (i > 0)
        {
            smart_str_appendl(&hosts, ",", 1);
        }

        smart_str_append(&hosts, Z_STR_P(host));
    }

    return smart_str_extract(&hosts);
}

static zend_always_inline void php_scylladb_parse_hosts(INTERNAL_FUNCTION_PARAMETERS, zend_string **out_hosts)
{
    zval *args = nullptr;
    int argc = 0;

    ZEND_PARSE_PARAMETERS_START(1, -1)
    Z_PARAM_VARIADIC('+', args, argc)
    ZEND_PARSE_PARAMETERS_END();

    zend_string *hosts = php_scylladb_build_hosts_str(args, argc);

    if (hosts == nullptr)
    {
        throw_invalid_argument(args, "hosts", "a string ip address or hostname");
        return;
    }

    if (*out_hosts != nullptr)
    {
        zend_string_release(*out_hosts);
    }

    *out_hosts = hosts;

    RETURN_ZVAL(getThis(), 1, 0);
}

static zend_always_inline void php_scylladb_set_timeout(INTERNAL_FUNCTION_PARAMETERS, uint32_t *out_timeout)
{
    double timeout = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_DOUBLE(timeout)
    ZEND_PARSE_PARAMETERS_END();

    if (timeout < 0)
    {
        zval val;
        ZVAL_DOUBLE(&val, timeout);
        throw_invalid_argument(&val, "timeout", "a positive number");
        return;
    }

    *out_timeout = (uint32_t)ceil(timeout * 1000);

    RETURN_ZVAL(getThis(), 1, 0);
}

/* Seconds, not milliseconds. cass_cluster_set_connection_heartbeat_interval and
 * cass_cluster_set_tcp_keepalive both take `unsigned *_secs`, so these fields
 * must not go through php_scylladb_set_timeout. */
static zend_always_inline void php_scylladb_set_interval_secs(INTERNAL_FUNCTION_PARAMETERS,
                                                              uint32_t *out_secs)
{
    double interval = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_DOUBLE(interval)
    ZEND_PARSE_PARAMETERS_END();

    if (interval < 0)
    {
        zval val;
        ZVAL_DOUBLE(&val, interval);
        throw_invalid_argument(&val, "interval", "a positive number");
        return;
    }

    *out_secs = (uint32_t)ceil(interval);

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, build)
{
#ifndef PHP_SCYLLADB_BACKEND_SCYLLA_RUST
    CassError rc;
#endif
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());

    object_init_ex(return_value, php_scylladb_default_cluster_ce);
    auto cluster = PHP_SCYLLADB_GET_CLUSTER(return_value);

    cluster->persist = self->persist;
    cluster->default_consistency = self->default_consistency;
    cluster->default_page_size = self->default_page_size;

    ZVAL_COPY(&cluster->default_timeout, &self->default_timeout);

    if (self->persist)
    {
        /* FNV-1a fingerprint of every field that affects the CassCluster
           identity. Persistent_list is integer-keyed → zero per-call
           allocation, even on cache hit. */
        zend_ulong h = php_scylladb_cache_key_init();
        h = php_scylladb_cache_key_mix_cstr(h, PHP_SCYLLADB_NAME);
        h = php_scylladb_cache_key_mix_zstr(h, self->contact_points);
        h = php_scylladb_cache_key_mix_int(h, self->port);
        h = php_scylladb_cache_key_mix_int(h, self->load_balancing_policy);
        h = php_scylladb_cache_key_mix_zstr(h, self->local_dc);
        h = php_scylladb_cache_key_mix_int(h, self->used_hosts_per_remote_dc);
        h = php_scylladb_cache_key_mix_int(h, self->allow_remote_dcs_for_local_cl);
        h = php_scylladb_cache_key_mix_int(h, self->use_token_aware_routing);
        h = php_scylladb_cache_key_mix_zstr(h, self->username);
        h = php_scylladb_cache_key_mix_zstr(h, self->password);
        h = php_scylladb_cache_key_mix_int(h, self->connect_timeout);
        h = php_scylladb_cache_key_mix_int(h, self->request_timeout);
        h = php_scylladb_cache_key_mix_int(h, self->protocol_version);
        h = php_scylladb_cache_key_mix_int(h, self->io_threads);
        h = php_scylladb_cache_key_mix_int(h, self->core_connections_per_host);
        h = php_scylladb_cache_key_mix_int(h, self->max_connections_per_host);
        h = php_scylladb_cache_key_mix_int(h, self->reconnect_interval);
        h = php_scylladb_cache_key_mix_int(h, self->enable_latency_aware_routing);
        h = php_scylladb_cache_key_mix_int(h, self->enable_tcp_nodelay);
        h = php_scylladb_cache_key_mix_int(h, self->enable_tcp_keepalive);
        h = php_scylladb_cache_key_mix_int(h, self->tcp_keepalive_delay);
        h = php_scylladb_cache_key_mix_int(h, self->enable_schema);
        h = php_scylladb_cache_key_mix_int(h, self->enable_hostname_resolution);
        h = php_scylladb_cache_key_mix_int(h, self->enable_randomized_contact_points);
        h = php_scylladb_cache_key_mix_int(h, self->connection_heartbeat_interval);
        h = php_scylladb_cache_key_mix_zstr(h, self->whitelist_hosts);
        h = php_scylladb_cache_key_mix_zstr(h, self->whitelist_dcs);
        h = php_scylladb_cache_key_mix_zstr(h, self->blacklist_hosts);
        h = php_scylladb_cache_key_mix_zstr(h, self->blacklist_dcs);
        h = php_scylladb_cache_key_mix_zstr(h, self->local_rack);
        h = php_scylladb_cache_key_mix_zstr(h, self->application_name);
        h = php_scylladb_cache_key_mix_zstr(h, self->application_version);
        h = php_scylladb_cache_key_mix_int(h, self->reconnect_policy);
        h = php_scylladb_cache_key_mix_int(h, self->reconnect_max_interval);
        h = php_scylladb_cache_key_mix_int(h, self->speculative_execution_delay);
        h = php_scylladb_cache_key_mix_int(h, self->speculative_execution_max);
        h = php_scylladb_cache_key_mix_int(h, self->coalesce_delay);
        h = php_scylladb_cache_key_mix_int(h, self->new_request_ratio);
        h = php_scylladb_cache_key_mix_zstr(h, self->local_address);
        h = php_scylladb_cache_key_mix_int(h, self->connection_idle_timeout);
        h = php_scylladb_cache_key_mix_int(h, self->max_schema_wait_time);
        h = php_scylladb_cache_key_mix_int(h, self->resolve_timeout);
        h = php_scylladb_cache_key_mix_int(h, self->monitor_reporting_interval);
        h = php_scylladb_cache_key_mix_int(h, self->queue_size_io);
        h = php_scylladb_cache_key_mix_int(h, self->tracing_consistency);
        h = php_scylladb_cache_key_mix_int(h, self->tracing_max_wait_time);
        h = php_scylladb_cache_key_mix_int(h, self->tracing_retry_wait_time);
        h = php_scylladb_cache_key_mix_int(h, self->prepare_on_all_hosts);
        h = php_scylladb_cache_key_mix_int(h, self->prepare_on_up_or_add_host);
        h = php_scylladb_cache_key_mix_int(h, self->shuffle_replicas);
        h = php_scylladb_cache_key_mix_int(h, self->no_compact);
        h = php_scylladb_cache_key_mix_int(h, self->beta_protocol);

        /* Each profile contributes its name and the running fingerprint of every
         * setter that was called on it. Without this, two clusters differing only
         * in a profile setting would share one cached CassCluster. */
        if (self->execution_profiles != nullptr)
        {
            zend_string *profile_name = nullptr;
            zval *profile_val = nullptr;
            ZEND_HASH_FOREACH_STR_KEY_VAL(self->execution_profiles, profile_name, profile_val)
            {
                h = php_scylladb_cache_key_mix_zstr(h, profile_name);
                h = php_scylladb_cache_key_mix_ulong(
                    h, PHP_SCYLLADB_GET_EXECUTION_PROFILE(profile_val)->config_hash);
            }
            ZEND_HASH_FOREACH_END();
        }

        cluster->cache_key = h;

        zval *le = zend_hash_index_find(&EG(persistent_list), h);
        if (le != nullptr && Z_TYPE_P(le) == IS_RESOURCE &&
            Z_RES_P(le)->type == php_le_php_scylladb_cluster())
        {
            cluster->cluster = (CassCluster *)Z_RES_P(le)->ptr;
            return; /* Return cached version */
        }

        /* Cache miss and the pool is full: build an uncached CassCluster and
         * let the object destructor own it, exactly as persist=false does. */
        if (!php_scylladb_persistent_can_cache(PHP_SCYLLADB_PERSISTENT_CLUSTERS))
        {
            cluster->persist = cass_false;
        }
    }

    cluster->cluster = cass_cluster_new();

    switch (self->load_balancing_policy)
    {
    case LOAD_BALANCING_DEFAULT:
        break;
    case LOAD_BALANCING_ROUND_ROBIN:
        cass_cluster_set_load_balance_round_robin(cluster->cluster);
        break;
    case LOAD_BALANCING_DC_AWARE_ROUND_ROBIN:
        ASSERT_SUCCESS(cass_cluster_set_load_balance_dc_aware(cluster->cluster, SAFE_ZEND_STRING(self->local_dc),
                                                              self->used_hosts_per_remote_dc,
                                                              self->allow_remote_dcs_for_local_cl));
        break;
    case LOAD_BALANCING_RACK_AWARE:
#ifdef PHP_SCYLLADB_BACKEND_CASSANDRA
        php_error_docref(nullptr, E_WARNING,
                         "The underlying C/C++ driver does not implement rack-aware load balancing; "
                         "the default policy will be used.");
#else
        /* ScyllaDB only. Empty strings make the driver infer the local
         * datacenter and rack from the first contact point it reaches. */
        cass_cluster_set_load_balance_rack_aware(cluster->cluster, SAFE_ZEND_STRING(self->local_dc),
                                                 SAFE_ZEND_STRING(self->local_rack));
#endif
        break;
    }

    if (self->blacklist_hosts != nullptr)
    {
        cass_cluster_set_blacklist_filtering(cluster->cluster, ZSTR_VAL(self->blacklist_hosts));
    }

    if (self->whitelist_hosts != nullptr)
    {
        cass_cluster_set_whitelist_filtering(cluster->cluster, ZSTR_VAL(self->whitelist_hosts));
    }

    if (self->blacklist_dcs != nullptr)
    {
        cass_cluster_set_blacklist_dc_filtering(cluster->cluster, ZSTR_VAL(self->blacklist_dcs));
    }

    if (self->whitelist_dcs != nullptr)
    {
        cass_cluster_set_whitelist_dc_filtering(cluster->cluster, ZSTR_VAL(self->whitelist_dcs));
    }

    cass_cluster_set_token_aware_routing(cluster->cluster, self->use_token_aware_routing);

    if (self->username != nullptr)
    {
        cass_cluster_set_credentials(cluster->cluster, ZSTR_VAL(self->username), ZSTR_VAL(self->password));
    }

    cass_cluster_set_connect_timeout(cluster->cluster, self->connect_timeout);
    cass_cluster_set_request_timeout(cluster->cluster, self->request_timeout);

    if (self->ssl_options != nullptr)
    {
        cass_cluster_set_ssl(cluster->cluster, self->ssl_options->ssl);
    }

    ASSERT_SUCCESS(cass_cluster_set_contact_points(cluster->cluster, ZSTR_VAL(self->contact_points)));
    ASSERT_SUCCESS(cass_cluster_set_port(cluster->cluster, self->port));

    ASSERT_SUCCESS(cass_cluster_set_protocol_version(cluster->cluster, self->protocol_version));
    ASSERT_SUCCESS(cass_cluster_set_num_threads_io(cluster->cluster, self->io_threads));
    ASSERT_SUCCESS(cass_cluster_set_core_connections_per_host(cluster->cluster, self->core_connections_per_host));
#ifndef PHP_SCYLLADB_BACKEND_SCYLLA_RUST
    ASSERT_SUCCESS(cass_cluster_set_max_connections_per_host(cluster->cluster, self->max_connections_per_host));
#endif
    if (self->reconnect_policy == RECONNECT_POLICY_EXPONENTIAL)
    {
        /* Backs off from reconnect_interval to reconnect_max_interval with
         * jitter, so a whole worker pool does not retry a recovering node in
         * lockstep the way a constant delay does. */
        ASSERT_SUCCESS(cass_cluster_set_exponential_reconnect(cluster->cluster, self->reconnect_interval,
                                                              self->reconnect_max_interval));
    }
    else
    {
        cass_cluster_set_constant_reconnect(cluster->cluster, self->reconnect_interval);
    }

    if (self->speculative_execution_delay > 0)
    {
        ASSERT_SUCCESS(cass_cluster_set_constant_speculative_execution_policy(
            cluster->cluster, self->speculative_execution_delay, self->speculative_execution_max));
    }

    ASSERT_SUCCESS(cass_cluster_set_coalesce_delay(cluster->cluster, self->coalesce_delay));

    cass_cluster_set_connection_idle_timeout(cluster->cluster, self->connection_idle_timeout);
    cass_cluster_set_max_schema_wait_time(cluster->cluster, self->max_schema_wait_time);
    cass_cluster_set_resolve_timeout(cluster->cluster, self->resolve_timeout);
    cass_cluster_set_token_aware_routing_shuffle_replicas(cluster->cluster, self->shuffle_replicas);
    cass_cluster_set_use_beta_protocol_version(cluster->cluster, self->beta_protocol);

    /* cpp-rs-driver lists all nine of these as UNIMPLEMENTED in its api.rs
     * manifest. Eight are not even declared, so calling them fails to compile.
     * prepare_on_all_hosts is declared but has no symbol in the library, which
     * fails later and louder: the extension loads and then dies with
     * "undefined symbol" on the first build(). Both need the same guard.
     * Every directive keeps its C driver default on that backend. */
#ifndef PHP_SCYLLADB_BACKEND_SCYLLA_RUST
    cass_cluster_set_prepare_on_all_hosts(cluster->cluster, self->prepare_on_all_hosts);
    ASSERT_SUCCESS(cass_cluster_set_new_request_ratio(cluster->cluster, self->new_request_ratio));
    ASSERT_SUCCESS(cass_cluster_set_queue_size_io(cluster->cluster, self->queue_size_io));
    cass_cluster_set_monitor_reporting_interval(cluster->cluster, self->monitor_reporting_interval);
    cass_cluster_set_prepare_on_up_or_add_host(cluster->cluster, self->prepare_on_up_or_add_host);
    cass_cluster_set_no_compact(cluster->cluster, self->no_compact);
    cass_cluster_set_tracing_consistency(cluster->cluster, (CassConsistency)self->tracing_consistency);
    cass_cluster_set_tracing_max_wait_time(cluster->cluster, self->tracing_max_wait_time);
    cass_cluster_set_tracing_retry_wait_time(cluster->cluster, self->tracing_retry_wait_time);
#endif

    if (self->local_address != nullptr)
    {
        ASSERT_SUCCESS(cass_cluster_set_local_address(cluster->cluster, ZSTR_VAL(self->local_address)));
    }

    if (self->execution_profiles != nullptr)
    {
        zend_string *profile_name = nullptr;
        zval *profile_val = nullptr;
        ZEND_HASH_FOREACH_STR_KEY_VAL(self->execution_profiles, profile_name, profile_val)
        {
            /* The cluster takes a copy, so the PHP object keeps owning its
               CassExecProfile and later edits do not reach this cluster. */
            ASSERT_SUCCESS(cass_cluster_set_execution_profile_n(
                cluster->cluster, ZSTR_VAL(profile_name), ZSTR_LEN(profile_name),
                PHP_SCYLLADB_GET_EXECUTION_PROFILE(profile_val)->profile));
        }
        ZEND_HASH_FOREACH_END();
    }

    if (self->application_name != nullptr)
    {
        cass_cluster_set_application_name_n(cluster->cluster, ZSTR_VAL(self->application_name),
                                            ZSTR_LEN(self->application_name));
    }

    if (self->application_version != nullptr)
    {
        cass_cluster_set_application_version_n(cluster->cluster, ZSTR_VAL(self->application_version),
                                               ZSTR_LEN(self->application_version));
    }
    cass_cluster_set_latency_aware_routing(cluster->cluster, self->enable_latency_aware_routing);
    cass_cluster_set_tcp_nodelay(cluster->cluster, self->enable_tcp_nodelay);
    cass_cluster_set_tcp_keepalive(cluster->cluster, self->enable_tcp_keepalive, self->tcp_keepalive_delay);
    cass_cluster_set_use_schema(cluster->cluster, self->enable_schema);

#ifdef PHP_SCYLLADB_BACKEND_SCYLLA_RUST
    if (self->enable_hostname_resolution)
    {
        php_error_docref(nullptr, E_WARNING,
                         "The underlying C/C++ driver does not implement hostname resolution; it will be disabled.");
    }
#else
    rc = cass_cluster_set_use_hostname_resolution(cluster->cluster, self->enable_hostname_resolution);
    if (rc == CASS_ERROR_LIB_NOT_IMPLEMENTED && self->enable_hostname_resolution)
    {
        php_error_docref(nullptr, E_WARNING,
                         "The underlying C/C++ driver does not implement hostname resolution; it will be disabled.");
    }
    else
    {
        ASSERT_SUCCESS(rc);
    }
#endif
    ASSERT_SUCCESS(
        cass_cluster_set_use_randomized_contact_points(cluster->cluster, self->enable_randomized_contact_points));
    cass_cluster_set_connection_heartbeat_interval(cluster->cluster, self->connection_heartbeat_interval);

    if (self->timestamp_gen != nullptr)
    {
        php_scylladb_timestamp_gen *timestamp_gen = self->timestamp_gen;
        cass_cluster_set_timestamp_gen(cluster->cluster, timestamp_gen->gen);
    }

    if (self->retry_policy != nullptr)
    {
        cass_cluster_set_retry_policy(cluster->cluster, self->retry_policy->policy);
    }

    if (cluster->persist)
    {
        zval resource;

        ZVAL_NEW_PERSISTENT_RES(&resource, 0, cluster->cluster, php_le_php_scylladb_cluster());

        (void)zend_hash_index_update(&EG(persistent_list), cluster->cache_key, &resource);
        PHP_SCYLLADB_G(persistent_clusters)++;
    }
}
ZEND_METHOD(Cassandra_Cluster_Builder, withDefaultConsistency)
{
    zend_long consistency;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(consistency)
    ZEND_PARSE_PARAMETERS_END();

    if (php_scylladb_validate_consistency((uint32_t)consistency) == -1)
    {
        zval consistency_val;
        ZVAL_LONG(&consistency_val, consistency);

        throw_invalid_argument(&consistency_val, "consistency", "one of " PHP_SCYLLADB_NAMESPACE "::CONSISTENCY_*");
        return;
    }

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->default_consistency = consistency;

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withDefaultPageSize)
{
    zend_long pageSize = -1;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(pageSize)
    ZEND_PARSE_PARAMETERS_END();

    if (pageSize < 0)
    {
        zval val;
        ZVAL_LONG(&val, pageSize);
        throw_invalid_argument(&val, "pageSize", "a positive integer or null");
        return;
    }

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->default_page_size = (int)pageSize;
    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withDefaultTimeout)
{
    double timeout = 0.0;
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_DOUBLE(timeout)
    ZEND_PARSE_PARAMETERS_END();

    zval val;
    ZVAL_DOUBLE(&val, timeout);

    if (timeout < 0.0)
    {
        throw_invalid_argument(&val, "timeout", "a positive number or null");
        return;
    }

    if (!Z_ISUNDEF(self->default_timeout))
    {
        zval_ptr_dtor(&self->default_timeout);
    }

    ZVAL_COPY_VALUE(&self->default_timeout, &val);

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withContactPoints)
{
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    php_scylladb_parse_hosts(INTERNAL_FUNCTION_PARAM_PASSTHRU, &self->contact_points);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withPort)
{
    zend_long port = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(port)
    ZEND_PARSE_PARAMETERS_END();

    if (port < 1 || port > 65535)
    {
        zval val;
        ZVAL_LONG(&val, port);
        throw_invalid_argument(&val, "port", "an integer between 1 and 65535");
        return;
    }

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->port = (int)port;
    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withRoundRobinLoadBalancingPolicy)
{
    ZEND_PARSE_PARAMETERS_NONE();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());

    if (self->local_dc)
    {
        zend_string_release(self->local_dc);
        self->local_dc = nullptr;
    }

    self->load_balancing_policy = LOAD_BALANCING_ROUND_ROBIN;

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withDatacenterAwareRoundRobinLoadBalancingPolicy)
{
    zend_string *local_dc;
    zend_long hostPerRemoteDatacenter = 0;
    zend_bool allow_remote_dcs_for_local_cl;

    ZEND_PARSE_PARAMETERS_START(3, 3)
    Z_PARAM_STR(local_dc)
    Z_PARAM_LONG(hostPerRemoteDatacenter)
    Z_PARAM_BOOL(allow_remote_dcs_for_local_cl)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());

    if (hostPerRemoteDatacenter < 0)
    {
        zval val;
        ZVAL_LONG(&val, hostPerRemoteDatacenter);
        throw_invalid_argument(&val, "hostPerRemoteDatacenter", "a positive integer");
        return;
    }

    if (self->local_dc)
    {
        zend_string_release(self->local_dc);
        self->local_dc = nullptr;
    }

    self->load_balancing_policy = LOAD_BALANCING_DC_AWARE_ROUND_ROBIN;
    self->local_dc = zend_string_copy(local_dc);
    self->used_hosts_per_remote_dc = hostPerRemoteDatacenter;
    self->allow_remote_dcs_for_local_cl = (cass_bool_t)(allow_remote_dcs_for_local_cl);

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withBlackListHosts)
{
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    php_scylladb_parse_hosts(INTERNAL_FUNCTION_PARAM_PASSTHRU, &self->blacklist_hosts);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withWhiteListHosts)
{
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    php_scylladb_parse_hosts(INTERNAL_FUNCTION_PARAM_PASSTHRU, &self->whitelist_hosts);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withBlackListDCs)
{
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    php_scylladb_parse_hosts(INTERNAL_FUNCTION_PARAM_PASSTHRU, &self->blacklist_dcs);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withWhiteListDCs)
{
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    php_scylladb_parse_hosts(INTERNAL_FUNCTION_PARAM_PASSTHRU, &self->whitelist_dcs);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withTokenAwareRouting)
{
    zend_bool enabled = true;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(enabled)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->use_token_aware_routing = (cass_bool_t)(enabled);

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withCredentials)
{
    zend_string *username = nullptr;
    zend_string *password = nullptr;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_STR(username)
    Z_PARAM_STR(password)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());

    if (self->username != nullptr)
    {
        zend_string_release(self->username);
    }
    if (self->password != nullptr)
    {
        zend_string_release(self->password);
    }

    self->username = zend_string_copy(username);
    self->password = zend_string_copy(password);

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withConnectTimeout)
{
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    php_scylladb_set_timeout(INTERNAL_FUNCTION_PARAM_PASSTHRU, &self->connect_timeout);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withRequestTimeout)
{
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    php_scylladb_set_timeout(INTERNAL_FUNCTION_PARAM_PASSTHRU, &self->request_timeout);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withSSL)
{
    zval *ssl_options = nullptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(ssl_options, php_scylladb_ssl_options_ce)
    ZEND_PARSE_PARAMETERS_END();

    auto ssl = Z_SCYLLADB_SSL_P(ssl_options);
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(ZEND_THIS);

    if (self->ssl_options != nullptr)
    {
        zend_object_release(&self->ssl_options->zendObject);
    }

    self->ssl_options = ssl;
    GC_ADDREF(&ssl->zendObject);
    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withPersistentSessions)
{
    zend_bool enabled = true;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(enabled)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    /* cassandra.allow_persistent=0 is an operator kill switch. Application
     * code may turn persistence off below it, never back on. */
    self->persist = (cass_bool_t)(enabled && PHP_SCYLLADB_G(allow_persistent));

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withProtocolVersion)
{
    zend_object *versionCase = nullptr;
    zend_long version = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJ_OF_CLASS_OR_LONG(versionCase, php_scylladb_protocol_version_ce, version)
    ZEND_PARSE_PARAMETERS_END();

    if (versionCase != nullptr)
    {
        version = Z_LVAL_P(zend_enum_fetch_case_value(versionCase));
    }
    else if (version < 1)
    {
        /* An int still gets through for versions the enum does not name, such
         * as the DSE ones — so it keeps its own range check. */
        zval val;
        ZVAL_LONG(&val, version);
        throw_invalid_argument(&val, "version", "a positive number");
        return;
    }

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->protocol_version = (uint32_t)version;

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withIOThreads)
{
    zend_long count;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(count)
    ZEND_PARSE_PARAMETERS_END();

    if (count < 1 || count > 128)
    {
        zval val;
        ZVAL_LONG(&val, count);
        throw_invalid_argument(&val, "count", "a number between 1 and 128");
        return;
    }

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->io_threads = (uint32_t)count;

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withConnectionsPerHost)
{
    zend_long core;
    zend_long max = 2;

    ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_LONG(core)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG(max)
    ZEND_PARSE_PARAMETERS_END();

    if (core > 128 || core < 1)
    {
        zval val;
        ZVAL_LONG(&val, core);
        throw_invalid_argument(&val, "core", "a number between 1 and 128");
        return;
    }

    if (max > 128 || max < 1)
    {
        zval val;
        ZVAL_LONG(&val, max);
        throw_invalid_argument(&val, "max", "a number between 1 and 128");
        return;
    }

    if (core > max) {
        zval val;
        ZVAL_LONG(&val, core);
        throw_invalid_argument(&val, "core", "greater than max");
        return;
    }

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->core_connections_per_host = core;
    self->max_connections_per_host = max;

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withReconnectInterval)
{
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    php_scylladb_set_timeout(INTERNAL_FUNCTION_PARAM_PASSTHRU, &self->reconnect_interval);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withLatencyAwareRouting)
{
    zend_bool enabled = true;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(enabled)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->enable_latency_aware_routing = (cass_bool_t)(enabled);

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withTCPNodelay)
{
    zend_bool enabled = true;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(enabled)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->enable_tcp_nodelay = (cass_bool_t)(enabled);

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withTCPKeepalive)
{
    double delay = 0;
    bool is_null = false;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_DOUBLE_OR_NULL(delay, is_null)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());

    if (is_null)
    {
        self->enable_tcp_keepalive = cass_false;
        self->tcp_keepalive_delay = 0;
        RETURN_ZVAL(getThis(), 1, 0);
    }

    if (delay < 0)
    {
        zval val;
        ZVAL_DOUBLE(&val, delay);
        throw_invalid_argument(&val, "delay", "a positive number or null");
        return;
    }

    self->enable_tcp_keepalive = cass_true;
    /* Seconds: cass_cluster_set_tcp_keepalive takes `unsigned delay_secs`. */
    self->tcp_keepalive_delay = (uint32_t)ceil(delay);

    RETURN_ZVAL(getThis(), 1, 0);
}
/* Replaces *dst with a copy of src, or with nullptr when src is empty. */
static zend_always_inline void php_scylladb_builder_set_str(zend_string **dst, zend_string *src)
{
    if (*dst != nullptr)
    {
        zend_string_release(*dst);
    }

    *dst = (src != nullptr && ZSTR_LEN(src) > 0) ? zend_string_copy(src) : nullptr;
}

ZEND_METHOD(Cassandra_Cluster_Builder, withRackAwareLoadBalancingPolicy)
{
    zend_string *local_dc = nullptr;
    zend_string *local_rack = nullptr;

    ZEND_PARSE_PARAMETERS_START(0, 2)
    Z_PARAM_OPTIONAL
    Z_PARAM_STR(local_dc)
    Z_PARAM_STR(local_rack)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());

    php_scylladb_builder_set_str(&self->local_dc, local_dc);
    php_scylladb_builder_set_str(&self->local_rack, local_rack);
    self->load_balancing_policy = LOAD_BALANCING_RACK_AWARE;

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withApplicationName)
{
    zend_string *name = nullptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    php_scylladb_builder_set_str(&PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis())->application_name, name);

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withApplicationVersion)
{
    zend_string *version = nullptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(version)
    ZEND_PARSE_PARAMETERS_END();

    php_scylladb_builder_set_str(&PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis())->application_version, version);

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withExponentialReconnect)
{
    double base_interval = 0.0;
    double max_interval = 0.0;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_DOUBLE(base_interval)
    Z_PARAM_DOUBLE(max_interval)
    ZEND_PARSE_PARAMETERS_END();

    if (base_interval <= 0)
    {
        zval val;
        ZVAL_DOUBLE(&val, base_interval);
        throw_invalid_argument(&val, "baseInterval", "a positive number");
        return;
    }

    if (max_interval < base_interval)
    {
        zval val;
        ZVAL_DOUBLE(&val, max_interval);
        throw_invalid_argument(&val, "maxInterval", "greater than or equal to the base interval");
        return;
    }

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->reconnect_policy = RECONNECT_POLICY_EXPONENTIAL;
    self->reconnect_interval = (uint32_t)ceil(base_interval * 1000);
    self->reconnect_max_interval = (uint32_t)ceil(max_interval * 1000);

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withConstantSpeculativeExecutionPolicy)
{
    double delay = 0.0;
    zend_long max_executions = 2;

    ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_DOUBLE(delay)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG(max_executions)
    ZEND_PARSE_PARAMETERS_END();

    if (delay <= 0)
    {
        zval val;
        ZVAL_DOUBLE(&val, delay);
        throw_invalid_argument(&val, "delay", "a positive number");
        return;
    }

    if (max_executions <= 0)
    {
        zval val;
        ZVAL_LONG(&val, max_executions);
        throw_invalid_argument(&val, "maxSpeculativeExecutions", "a positive integer");
        return;
    }

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->speculative_execution_delay = (uint32_t)ceil(delay * 1000);
    self->speculative_execution_max = (int32_t)max_executions;

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withNoSpeculativeExecutionPolicy)
{
    ZEND_PARSE_PARAMETERS_NONE();

    /* Zero delay is what build() reads as "leave the driver default policy". */
    PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis())->speculative_execution_delay = 0;

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withCoalesceDelay)
{
    zend_long delay_us = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(delay_us)
    ZEND_PARSE_PARAMETERS_END();

    if (delay_us <= 0)
    {
        zval val;
        ZVAL_LONG(&val, delay_us);
        throw_invalid_argument(&val, "microseconds", "a positive integer");
        return;
    }

    PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis())->coalesce_delay = (int32_t)delay_us;

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withNewRequestRatio)
{
    zend_long ratio = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(ratio)
    ZEND_PARSE_PARAMETERS_END();

    /* cass_cluster_set_new_request_ratio documents a range of 1 to 100. */
    if (ratio < 1 || ratio > 100)
    {
        zval val;
        ZVAL_LONG(&val, ratio);
        throw_invalid_argument(&val, "ratio", "between 1 and 100");
        return;
    }

    PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis())->new_request_ratio = (int32_t)ratio;

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withExecutionProfile)
{
    zval *name_zv = nullptr;
    zval *profile = nullptr;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_ZVAL(name_zv)
    Z_PARAM_OBJECT_OF_CLASS(profile, php_scylladb_execution_profile_ce)
    ZEND_PARSE_PARAMETERS_END();

    zend_string *name = php_scylladb_name_from_string_or_enum(name_zv);

    if (name == nullptr)
    {
        throw_invalid_argument(name_zv, "name", "a string or an enum case");
        return;
    }

    if (ZSTR_LEN(name) == 0)
    {
        zval val;
        ZVAL_STR_COPY(&val, name);
        zend_string_release(name);
        throw_invalid_argument(&val, "name", "a non-empty string");
        zval_ptr_dtor(&val);
        return;
    }

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());

    if (self->execution_profiles == nullptr)
    {
        ALLOC_HASHTABLE(self->execution_profiles);
        zend_hash_init(self->execution_profiles, 4, nullptr, ZVAL_PTR_DTOR, 0);
    }

    Z_ADDREF_P(profile);
    (void)zend_hash_update(self->execution_profiles, name, profile);
    zend_string_release(name);

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withRetryPolicy)
{
    zval *retry_policy = nullptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(retry_policy, php_scylladb_retry_policy_ce)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    php_scylladb_retry_policy *policy = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_retry_policy, Z_OBJ_P(retry_policy));

    if (self->retry_policy != nullptr)
    {
        zend_object_release(&self->retry_policy->zendObject);
    }

    self->retry_policy = policy;
    GC_ADDREF(&policy->zendObject);

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withTimestampGenerator)
{
    zval *timestamp_gen = nullptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(timestamp_gen, php_scylladb_timestamp_generator_ce)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());

    if (self->timestamp_gen != nullptr)
    {
        zend_object_release(&self->timestamp_gen->zendObject);
    }

    auto timestamp_generator = php_scylladb_timestamp_gen_object_fetch(Z_OBJ_P(timestamp_gen));
    self->timestamp_gen = timestamp_generator;
    GC_ADDREF(&timestamp_generator->zendObject);

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withSchemaMetadata)
{
    zend_bool enabled = true;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(enabled)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->enable_schema = (cass_bool_t)(enabled);

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withHostnameResolution)
{
    zend_bool enabled = true;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(enabled)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    self->enable_hostname_resolution = (cass_bool_t)(enabled);

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withRandomizedContactPoints)
{
    zend_bool enabled = true;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(enabled)
    ZEND_PARSE_PARAMETERS_END();

    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());

    self->enable_randomized_contact_points = (cass_bool_t)(enabled);

    RETURN_ZVAL(getThis(), 1, 0);
}
ZEND_METHOD(Cassandra_Cluster_Builder, withConnectionHeartbeatInterval)
{
    auto self = PHP_SCYLLADB_GET_CLUSTER_BUILDER(getThis());
    php_scylladb_set_interval_secs(INTERNAL_FUNCTION_PARAM_PASSTHRU, &self->connection_heartbeat_interval);
}
