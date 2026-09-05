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

#include <math.h>
#include <zend_smart_str.h>

#include "php_scylladb.h"
#include "php_scylladb_cache_key.h"
#include "php_scylladb_consistency.h"
#include "php_scylladb_types.h"

#include "ExecutionProfile_arginfo.h"

extern zend_object_handlers php_scylladb_execution_profile_handlers;

/* Every setter folds its arguments into config_hash. Cluster/Builder.c mixes that
 * into the persistent-cluster cache key, so two clusters that differ only in a
 * profile setting do not collide on one cached CassCluster. */
#define PHP_SCYLLADB_PROFILE_MIX_INT(self, v) \
    ((self)->config_hash = php_scylladb_cache_key_mix_int((self)->config_hash, (int)(v)))

static zend_always_inline php_scylladb_execution_profile *php_scylladb_profile_this(zval *obj, const char *tag)
{
    php_scylladb_execution_profile *self = PHP_SCYLLADB_GET_EXECUTION_PROFILE(obj);
    self->config_hash = php_scylladb_cache_key_mix_cstr(self->config_hash, tag);
    return self;
}

/* Joins variadic string arguments with commas, as the filtering setters expect. */
static zend_always_inline zend_string *php_scylladb_profile_join(const zval *args, const size_t argc)
{
    smart_str out = (smart_str){nullptr, 0};

    for (size_t i = 0; i < argc; i++)
    {
        if (Z_TYPE(args[i]) != IS_STRING)
        {
            smart_str_free(&out);
            return nullptr;
        }

        if (i > 0)
        {
            smart_str_appendl(&out, ",", 1);
        }

        smart_str_append(&out, Z_STR(args[i]));
    }

    return smart_str_extract(&out);
}

/* Shared body for the four whitelist/blacklist setters. */
static zend_always_inline void php_scylladb_profile_filter(INTERNAL_FUNCTION_PARAMETERS, const char *tag,
                                                           CassError (*apply)(CassExecProfile *, const char *))
{
    zval *args = nullptr;
    uint32_t argc = 0;

    ZEND_PARSE_PARAMETERS_START(1, -1)
    Z_PARAM_VARIADIC('+', args, argc)
    ZEND_PARSE_PARAMETERS_END();

    zend_string *joined = php_scylladb_profile_join(args, (size_t)argc);

    if (joined == nullptr)
    {
        throw_invalid_argument(args, "hosts", "a string ip address, hostname or datacenter name");
        return;
    }

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, tag);
    self->config_hash = php_scylladb_cache_key_mix_zstr(self->config_hash, joined);

    const CassError rc = apply(self->profile, ZSTR_VAL(joined));
    zend_string_release(joined);

    ASSERT_SUCCESS(rc);

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, __construct) { ZEND_PARSE_PARAMETERS_NONE(); }

ZEND_METHOD(Cassandra_ExecutionProfile, withConsistency)
{
    zend_long consistency = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(consistency)
    ZEND_PARSE_PARAMETERS_END();

    if (php_scylladb_validate_consistency((uint32_t)consistency) == -1)
    {
        zval val;
        ZVAL_LONG(&val, consistency);
        throw_invalid_argument(&val, "consistency", "one of " PHP_SCYLLADB_NAMESPACE "::CONSISTENCY_*");
        return;
    }

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "consistency");
    PHP_SCYLLADB_PROFILE_MIX_INT(self, consistency);

    ASSERT_SUCCESS(cass_execution_profile_set_consistency(self->profile, (CassConsistency)consistency));

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withSerialConsistency)
{
    zend_long consistency = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(consistency)
    ZEND_PARSE_PARAMETERS_END();

    if (php_scylladb_validate_serial_consistency((uint32_t)consistency) == -1)
    {
        zval val;
        ZVAL_LONG(&val, consistency);
        throw_invalid_argument(&val, "consistency",
                               PHP_SCYLLADB_NAMESPACE "::CONSISTENCY_SERIAL or " PHP_SCYLLADB_NAMESPACE
                                                      "::CONSISTENCY_LOCAL_SERIAL");
        return;
    }

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "serial_consistency");
    PHP_SCYLLADB_PROFILE_MIX_INT(self, consistency);

    ASSERT_SUCCESS(cass_execution_profile_set_serial_consistency(self->profile, (CassConsistency)consistency));

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withRequestTimeout)
{
    double timeout = 0.0;

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

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "request_timeout");
    const double timeout_scaled = ceil(timeout * 1000);
    const cass_uint64_t timeout_ms = (cass_uint64_t)timeout_scaled;
    PHP_SCYLLADB_PROFILE_MIX_INT(self, timeout_ms);

    ASSERT_SUCCESS(cass_execution_profile_set_request_timeout(self->profile, timeout_ms));

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withRetryPolicy)
{
    zval *policy = nullptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(policy, php_scylladb_retry_policy_ce)
    ZEND_PARSE_PARAMETERS_END();

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "retry_policy");
    php_scylladb_retry_policy *rp = PHP_SCYLLADB_OBJ_FETCH(php_scylladb_retry_policy, Z_OBJ_P(policy));

    /* The profile borrows the CassRetryPolicy, so hold a reference to the PHP
     * object for as long as this profile lives. */
    if (self->retry_policy != nullptr)
    {
        zend_object_release(&self->retry_policy->zendObject);
    }
    GC_ADDREF(&rp->zendObject);
    self->retry_policy = rp;

    self->config_hash = php_scylladb_cache_key_mix_int(self->config_hash, (int)(uintptr_t)rp->policy);

    ASSERT_SUCCESS(cass_execution_profile_set_retry_policy(self->profile, rp->policy));

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withRoundRobinLoadBalancingPolicy)
{
    ZEND_PARSE_PARAMETERS_NONE();

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "lb_round_robin");

    ASSERT_SUCCESS(cass_execution_profile_set_load_balance_round_robin(self->profile));

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withDatacenterAwareRoundRobinLoadBalancingPolicy)
{
    zend_string *local_dc = nullptr;
    zend_long hosts_per_remote_dc = 0;
    bool allow_remote_for_local = false;

    ZEND_PARSE_PARAMETERS_START(3, 3)
    Z_PARAM_STR(local_dc)
    Z_PARAM_LONG(hosts_per_remote_dc)
    Z_PARAM_BOOL(allow_remote_for_local)
    ZEND_PARSE_PARAMETERS_END();

    if (hosts_per_remote_dc < 0)
    {
        zval val;
        ZVAL_LONG(&val, hosts_per_remote_dc);
        throw_invalid_argument(&val, "hostPerRemoteDatacenter", "zero or a positive integer");
        return;
    }

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "lb_dc_aware");
    self->config_hash = php_scylladb_cache_key_mix_zstr(self->config_hash, local_dc);
    PHP_SCYLLADB_PROFILE_MIX_INT(self, hosts_per_remote_dc);
    PHP_SCYLLADB_PROFILE_MIX_INT(self, allow_remote_for_local);

    ASSERT_SUCCESS(cass_execution_profile_set_load_balance_dc_aware_n(
        self->profile, ZSTR_VAL(local_dc), ZSTR_LEN(local_dc), (unsigned)hosts_per_remote_dc,
        allow_remote_for_local ? cass_true : cass_false));

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withTokenAwareRouting)
{
    bool enabled = true;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(enabled)
    ZEND_PARSE_PARAMETERS_END();

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "token_aware");
    PHP_SCYLLADB_PROFILE_MIX_INT(self, enabled);

    ASSERT_SUCCESS(
        cass_execution_profile_set_token_aware_routing(self->profile, enabled ? cass_true : cass_false));

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withTokenAwareRoutingShuffleReplicas)
{
    bool enabled = true;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(enabled)
    ZEND_PARSE_PARAMETERS_END();

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "shuffle_replicas");
    PHP_SCYLLADB_PROFILE_MIX_INT(self, enabled);

    ASSERT_SUCCESS(cass_execution_profile_set_token_aware_routing_shuffle_replicas(
        self->profile, enabled ? cass_true : cass_false));

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withLatencyAwareRouting)
{
    bool enabled = true;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_BOOL(enabled)
    ZEND_PARSE_PARAMETERS_END();

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "latency_aware");
    PHP_SCYLLADB_PROFILE_MIX_INT(self, enabled);

    ASSERT_SUCCESS(
        cass_execution_profile_set_latency_aware_routing(self->profile, enabled ? cass_true : cass_false));

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withLatencyAwareRoutingSettings)
{
    double exclusion_threshold = 0.0;
    double scale = 0.0;
    double retry_period = 0.0;
    double update_rate = 0.0;
    zend_long min_measured = 0;

    ZEND_PARSE_PARAMETERS_START(5, 5)
    Z_PARAM_DOUBLE(exclusion_threshold)
    Z_PARAM_DOUBLE(scale)
    Z_PARAM_DOUBLE(retry_period)
    Z_PARAM_DOUBLE(update_rate)
    Z_PARAM_LONG(min_measured)
    ZEND_PARSE_PARAMETERS_END();

    if (exclusion_threshold < 0 || scale < 0 || retry_period < 0 || update_rate < 0 || min_measured < 0)
    {
        zval val;
        ZVAL_DOUBLE(&val, exclusion_threshold);
        throw_invalid_argument(&val, "latency aware routing settings", "positive numbers");
        return;
    }

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "latency_aware_settings");
    const double scale_scaled = ceil(scale * 1000);
    const double retry_period_scaled = ceil(retry_period * 1000);
    const double update_rate_scaled = ceil(update_rate * 1000);
    const cass_uint64_t scale_ms = (cass_uint64_t)scale_scaled;
    const cass_uint64_t retry_period_ms = (cass_uint64_t)retry_period_scaled;
    const cass_uint64_t update_rate_ms = (cass_uint64_t)update_rate_scaled;

    PHP_SCYLLADB_PROFILE_MIX_INT(self, (int)(exclusion_threshold * 1000));
    PHP_SCYLLADB_PROFILE_MIX_INT(self, scale_ms);
    PHP_SCYLLADB_PROFILE_MIX_INT(self, retry_period_ms);
    PHP_SCYLLADB_PROFILE_MIX_INT(self, update_rate_ms);
    PHP_SCYLLADB_PROFILE_MIX_INT(self, min_measured);

    /* Deliberately not ASSERT_SUCCESS. cpp-rs-driver declares this returning
     * CassError but its Rust implementation returns nothing, so the caller
     * reads whatever happens to be in the return register and a check would
     * throw at random. The setter only stores fields and cannot fail for a
     * profile we just allocated, on either driver. */
    (void)cass_execution_profile_set_latency_aware_routing_settings(
        self->profile, exclusion_threshold, scale_ms, retry_period_ms, update_rate_ms,
        (cass_uint64_t)min_measured);

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withConstantSpeculativeExecutionPolicy)
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

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "speculative");
    const double delay_scaled = ceil(delay * 1000);
    const cass_int64_t delay_ms = (cass_int64_t)delay_scaled;
    PHP_SCYLLADB_PROFILE_MIX_INT(self, delay_ms);
    PHP_SCYLLADB_PROFILE_MIX_INT(self, max_executions);

    ASSERT_SUCCESS(cass_execution_profile_set_constant_speculative_execution_policy(self->profile, delay_ms,
                                                                                   (int)max_executions));

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withNoSpeculativeExecutionPolicy)
{
    ZEND_PARSE_PARAMETERS_NONE();

    php_scylladb_execution_profile *self = php_scylladb_profile_this(ZEND_THIS, "no_speculative");

    ASSERT_SUCCESS(cass_execution_profile_set_no_speculative_execution_policy(self->profile));

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withWhiteListHosts)
{
    php_scylladb_profile_filter(INTERNAL_FUNCTION_PARAM_PASSTHRU, "whitelist_hosts",
                                cass_execution_profile_set_whitelist_filtering);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withBlackListHosts)
{
    php_scylladb_profile_filter(INTERNAL_FUNCTION_PARAM_PASSTHRU, "blacklist_hosts",
                                cass_execution_profile_set_blacklist_filtering);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withWhiteListDCs)
{
    php_scylladb_profile_filter(INTERNAL_FUNCTION_PARAM_PASSTHRU, "whitelist_dcs",
                                cass_execution_profile_set_whitelist_dc_filtering);
}

ZEND_METHOD(Cassandra_ExecutionProfile, withBlackListDCs)
{
    php_scylladb_profile_filter(INTERNAL_FUNCTION_PARAM_PASSTHRU, "blacklist_dcs",
                                cass_execution_profile_set_blacklist_dc_filtering);
}

void php_scylladb_execution_profile_free(zend_object *object)
{
    php_scylladb_execution_profile *self = php_scylladb_execution_profile_object_fetch(object);

    if (self->profile != nullptr)
    {
        cass_execution_profile_free(self->profile);
        self->profile = nullptr;
    }

    if (self->retry_policy != nullptr)
    {
        zend_object_release(&self->retry_policy->zendObject);
        self->retry_policy = nullptr;
    }

    zend_object_std_dtor(&self->zendObject);
}

HashTable *php_scylladb_execution_profile_gc(zend_object *object, zval **table, int *n)
{
    /* No zvals are held, so hand the collector an empty buffer rather than let
     * zend_std_get_gc call the side-effecting get_properties. */
    (void)object;
    zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();
    zend_get_gc_buffer_use(buffer, table, n);
    return nullptr;
}

zend_object *php_scylladb_execution_profile_new(zend_class_entry *ce)
{
    php_scylladb_execution_profile *self =
        PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_execution_profile, ce, &php_scylladb_execution_profile_handlers);

    self->profile = cass_execution_profile_new();
    self->config_hash = php_scylladb_cache_key_init();
    self->retry_policy = nullptr;

    return &self->zendObject;
}
