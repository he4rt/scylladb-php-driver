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

#include <cassandra_driver.h>
#include <zend_smart_str.h>

#include "php_driver.h"
#include "php_driver_globals.h"
#include "php_driver_types.h"
#include "util/consistency.h"

#include <Cluster/Cluster.h>

#include "Builder.h"
#include "Builder_arginfo.h"

zend_class_entry* php_driver_cluster_builder_ce = NULL;

ZEND_METHOD(Cassandra_Cluster_Builder, build)
{
  CassError rc;

  object_init_ex(return_value, php_driver_default_cluster_ce);
  php_driver_cluster* cluster      = PHP_DRIVER_CLUSTER_ZVAL_TO_OBJECT(return_value);
  php_driver_cluster_builder* self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  cluster->persist             = self->persist;
  cluster->default_consistency = self->default_consistency;
  cluster->default_page_size   = self->default_page_size;

  PHP5TO7_ZVAL_COPY(&cluster->default_timeout, &self->default_timeout);

  if (self->persist) {
    cluster->hash_key_len = spprintf(&cluster->hash_key, 0,
                                     PHP_DRIVER_NAME ":%s:%d:%d:%s:%d:%d:%d:%s:%s:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%s:%s:%s:%s",
                                     self->contact_points, self->port, self->load_balancing_policy,
                                     SAFE_STR(self->local_dc), self->used_hosts_per_remote_dc,
                                     self->allow_remote_dcs_for_local_cl, self->use_token_aware_routing,
                                     SAFE_STR(self->username), SAFE_STR(self->password),
                                     self->connect_timeout, self->request_timeout,
                                     self->protocol_version, self->io_threads,
                                     self->core_connections_per_host, self->max_connections_per_host,
                                     self->reconnect_interval, self->enable_latency_aware_routing,
                                     self->enable_tcp_nodelay, self->enable_tcp_keepalive,
                                     self->tcp_keepalive_delay, self->enable_schema,
                                     self->enable_hostname_resolution, self->enable_randomized_contact_points,
                                     self->connection_heartbeat_interval,
                                     SAFE_STR(self->whitelist_hosts), SAFE_STR(self->whitelist_dcs),
                                     SAFE_STR(self->blacklist_hosts), SAFE_STR(self->blacklist_dcs));

    if (self->persist) {
      zval* le;

      if (PHP5TO7_ZEND_HASH_FIND(&EG(persistent_list), cluster->hash_key, cluster->hash_key_len + 1, le)) {
        if (Z_TYPE_P(le) == php_le_php_driver_cluster()) {
          cluster->cluster = (CassCluster*) Z_RES_P(le)->ptr;
          return; /* Return cached version */
        }
      }
    }
  }

  cluster->cluster = cass_cluster_new();

  if (self->load_balancing_policy == LOAD_BALANCING_ROUND_ROBIN) {
    cass_cluster_set_load_balance_round_robin(cluster->cluster);
  }

  if (self->load_balancing_policy == LOAD_BALANCING_DC_AWARE_ROUND_ROBIN) {
    ASSERT_SUCCESS(cass_cluster_set_load_balance_dc_aware(cluster->cluster,
                                                          self->local_dc,
                                                          self->used_hosts_per_remote_dc,
                                                          self->allow_remote_dcs_for_local_cl));
  }

  if (self->blacklist_hosts != NULL) {
    cass_cluster_set_blacklist_filtering(cluster->cluster, self->blacklist_hosts);
  }

  if (self->whitelist_hosts != NULL) {
    cass_cluster_set_whitelist_filtering(cluster->cluster, self->whitelist_hosts);
  }

  if (self->blacklist_dcs != NULL) {
    cass_cluster_set_blacklist_dc_filtering(cluster->cluster, self->blacklist_dcs);
  }

  if (self->whitelist_dcs != NULL) {
    cass_cluster_set_whitelist_dc_filtering(cluster->cluster, self->whitelist_dcs);
  }

  cass_cluster_set_token_aware_routing(cluster->cluster, self->use_token_aware_routing);

  if (self->username) {
    cass_cluster_set_credentials(cluster->cluster, self->username, self->password);
  }

  cass_cluster_set_connect_timeout(cluster->cluster, self->connect_timeout);
  cass_cluster_set_request_timeout(cluster->cluster, self->request_timeout);

  if (!PHP5TO7_ZVAL_IS_UNDEF(self->ssl_options)) {
    php_driver_ssl* options = PHP_DRIVER_GET_SSL(PHP5TO7_ZVAL_MAYBE_P(self->ssl_options));
    cass_cluster_set_ssl(cluster->cluster, options->ssl);
  }

  ASSERT_SUCCESS(cass_cluster_set_contact_points(cluster->cluster, self->contact_points));
  ASSERT_SUCCESS(cass_cluster_set_port(cluster->cluster, self->port));

  ASSERT_SUCCESS(cass_cluster_set_protocol_version(cluster->cluster, self->protocol_version));
  ASSERT_SUCCESS(cass_cluster_set_num_threads_io(cluster->cluster, self->io_threads));
  ASSERT_SUCCESS(cass_cluster_set_core_connections_per_host(cluster->cluster, self->core_connections_per_host));
  cass_cluster_set_reconnect_wait_time(cluster->cluster, self->reconnect_interval);
  cass_cluster_set_latency_aware_routing(cluster->cluster, self->enable_latency_aware_routing);
  cass_cluster_set_tcp_nodelay(cluster->cluster, self->enable_tcp_nodelay);
  cass_cluster_set_tcp_keepalive(cluster->cluster, self->enable_tcp_keepalive, self->tcp_keepalive_delay);
  cass_cluster_set_use_schema(cluster->cluster, self->enable_schema);

  rc = cass_cluster_set_use_hostname_resolution(cluster->cluster, self->enable_hostname_resolution);
  if (rc == CASS_ERROR_LIB_NOT_IMPLEMENTED) {
    if (self->enable_hostname_resolution) {
      php_error_docref(NULL, E_WARNING,
                       "The underlying C/C++ driver does not implement hostname resolution it will be disabled");
    }
  } else {
    ASSERT_SUCCESS(rc);
  }
  ASSERT_SUCCESS(cass_cluster_set_use_randomized_contact_points(cluster->cluster,
                                                                self->enable_randomized_contact_points));
  cass_cluster_set_connection_heartbeat_interval(cluster->cluster, self->connection_heartbeat_interval);

  if (!Z_ISUNDEF(self->timestamp_gen)) {
    php_driver_timestamp_gen* timestamp_gen =
      PHP_DRIVER_GET_TIMESTAMP_GEN(&self->timestamp_gen);
    cass_cluster_set_timestamp_gen(cluster->cluster, timestamp_gen->gen);
  }

  if (!Z_ISUNDEF(self->retry_policy)) {
    php_driver_retry_policy* retry_policy =
      PHP_DRIVER_GET_RETRY_POLICY(&self->retry_policy);
    cass_cluster_set_retry_policy(cluster->cluster, retry_policy->policy);
  }

  if (self->persist) {
    zval resource;

    ZVAL_NEW_PERSISTENT_RES(&resource, 0, cluster->cluster, php_le_php_driver_cluster());

    PHP5TO7_ZEND_HASH_UPDATE(&EG(persistent_list),
                             cluster->hash_key, cluster->hash_key_len + 1,
                             &resource, sizeof(zval));
    PHP_DRIVER_G(persistent_clusters)
    ++;
  }
}

ZEND_METHOD(Cassandra_Cluster_Builder, withDefaultConsistency)
{
  zval* consistency = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &consistency) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (php_driver_get_consistency(consistency, &self->default_consistency) == FAILURE) {
    return;
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withDefaultPageSize)
{
  zval* pageSize = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &pageSize) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(pageSize) == IS_NULL) {
    self->default_page_size = -1;
  } else if (Z_TYPE_P(pageSize) == IS_LONG && Z_LVAL_P(pageSize) > 0) {
    self->default_page_size = Z_LVAL_P(pageSize);
  } else {
    INVALID_ARGUMENT(pageSize, "a positive integer or null");
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withDefaultTimeout)
{
  zval* timeout = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &timeout) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(timeout) == IS_NULL) {
    ZVAL_DESTROY(self->default_timeout);
    ZVAL_UNDEF(&self->default_timeout);
  } else if ((Z_TYPE_P(timeout) == IS_LONG && Z_LVAL_P(timeout) > 0) || (Z_TYPE_P(timeout) == IS_DOUBLE && Z_LVAL_P(timeout) > 0)) {
    ZVAL_DESTROY(self->default_timeout);
    PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(self->default_timeout), timeout);
  } else {
    INVALID_ARGUMENT(timeout, "a number of seconds greater than zero or null");
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withContactPoints)
{
  zval* host              = NULL;
  zval* args              = NULL;
  int argc                = 0, i;
  smart_str contactPoints = PHP5TO7_SMART_STR_INIT;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  for (i = 0; i < argc; i++) {
    host = PHP5TO7_ZVAL_ARG(args[i]);

    if (Z_TYPE_P(host) != IS_STRING) {
      smart_str_free(&contactPoints);
      throw_invalid_argument(host, "host", "a string ip address or hostname");
      return;
    }

    if (i > 0) {
      smart_str_appendl(&contactPoints, ",", 1);
    }

    smart_str_appendl(&contactPoints, Z_STRVAL_P(host), Z_STRLEN_P(host));
  }

  smart_str_0(&contactPoints);

  efree(self->contact_points);
  self->contact_points = estrndup(contactPoints.s->val, contactPoints.s->len);
  smart_str_free(&contactPoints);

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withPort)
{
  zval* port = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &port) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(port) == IS_LONG && Z_LVAL_P(port) > 0 && Z_LVAL_P(port) < 65536) {
    self->port = Z_LVAL_P(port);
  } else {
    INVALID_ARGUMENT(port, "an integer between 1 and 65535");
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withRoundRobinLoadBalancingPolicy)
{
  php_driver_cluster_builder* self;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (self->local_dc) {
    efree(self->local_dc);
    self->local_dc = NULL;
  }

  self->load_balancing_policy = LOAD_BALANCING_ROUND_ROBIN;

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withDatacenterAwareRoundRobinLoadBalancingPolicy)
{
  char* local_dc;
  size_t local_dc_len;
  zval* hostPerRemoteDatacenter = NULL;
  zend_bool allow_remote_dcs_for_local_cl;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(
        ZEND_NUM_ARGS(),
        "szb",
        &local_dc,
        &local_dc_len,
        &hostPerRemoteDatacenter,
        &allow_remote_dcs_for_local_cl)
      == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(hostPerRemoteDatacenter) != IS_LONG || Z_LVAL_P(hostPerRemoteDatacenter) < 0) {
    INVALID_ARGUMENT(hostPerRemoteDatacenter, "a positive integer");
  }

  if (self->local_dc) {
    efree(self->local_dc);
    self->local_dc = NULL;
  }

  self->load_balancing_policy         = LOAD_BALANCING_DC_AWARE_ROUND_ROBIN;
  self->local_dc                      = estrndup(local_dc, local_dc_len);
  self->used_hosts_per_remote_dc      = Z_LVAL_P(hostPerRemoteDatacenter);
  self->allow_remote_dcs_for_local_cl = allow_remote_dcs_for_local_cl;

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withBlackListHosts)
{
  zval* hosts               = NULL;
  zval* args                = NULL;
  int argc                  = 0, i;
  smart_str blacklist_hosts = PHP5TO7_SMART_STR_INIT;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  for (i = 0; i < argc; i++) {
    hosts = PHP5TO7_ZVAL_ARG(args[i]);

    if (Z_TYPE_P(hosts) != IS_STRING) {
      smart_str_free(&blacklist_hosts);
      throw_invalid_argument(hosts, "hosts", "a string ip address or hostname");
      return;
    }

    if (i > 0) {
      smart_str_appendl(&blacklist_hosts, ",", 1);
    }

    smart_str_appendl(&blacklist_hosts, Z_STRVAL_P(hosts), Z_STRLEN_P(hosts));
  }

  smart_str_0(&blacklist_hosts);

  efree(self->blacklist_hosts);
  self->blacklist_hosts = estrndup(blacklist_hosts.s->val, blacklist_hosts.s->len);
  smart_str_free(&blacklist_hosts);

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withWhiteListHosts)
{
  zval* hosts               = NULL;
  zval* args                = NULL;
  int argc                  = 0, i;
  smart_str whitelist_hosts = PHP5TO7_SMART_STR_INIT;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  for (i = 0; i < argc; i++) {
    hosts = PHP5TO7_ZVAL_ARG(args[i]);

    if (Z_TYPE_P(hosts) != IS_STRING) {
      smart_str_free(&whitelist_hosts);
      throw_invalid_argument(hosts, "hosts", "a string ip address or hostname");
      return;
    }

    if (i > 0) {
      smart_str_appendl(&whitelist_hosts, ",", 1);
    }

    smart_str_appendl(&whitelist_hosts, Z_STRVAL_P(hosts), Z_STRLEN_P(hosts));
  }

  smart_str_0(&whitelist_hosts);

  efree(self->whitelist_hosts);
  self->whitelist_hosts = estrndup(whitelist_hosts.s->val, whitelist_hosts.s->len);
  smart_str_free(&whitelist_hosts);

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withBlackListDCs)
{
  zval* dcs               = NULL;
  zval* args              = NULL;
  int argc                = 0, i;
  smart_str blacklist_dcs = PHP5TO7_SMART_STR_INIT;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  for (i = 0; i < argc; i++) {
    dcs = PHP5TO7_ZVAL_ARG(args[i]);

    if (Z_TYPE_P(dcs) != IS_STRING) {
      smart_str_free(&blacklist_dcs);
      throw_invalid_argument(dcs, "dcs", "a string");

      return;
    }

    if (i > 0) {
      smart_str_appendl(&blacklist_dcs, ",", 1);
    }

    smart_str_appendl(&blacklist_dcs, Z_STRVAL_P(dcs), Z_STRLEN_P(dcs));
  }

  smart_str_0(&blacklist_dcs);

  efree(self->blacklist_dcs);
#if PHP_MAJOR_VERSION >= 7
  self->blacklist_dcs = estrndup(blacklist_dcs.s->val, blacklist_dcs.s->len);
  smart_str_free(&blacklist_dcs);
#else
  self->blacklist_dcs = blacklist_dcs.c;
#endif

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withWhiteListDCs)
{
  zval* dcs               = NULL;
  zval* args              = NULL;
  int argc                = 0, i;
  smart_str whitelist_dcs = PHP5TO7_SMART_STR_INIT;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &argc) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  for (i = 0; i < argc; i++) {
    dcs = PHP5TO7_ZVAL_ARG(args[i]);

    if (Z_TYPE_P(dcs) != IS_STRING) {
      smart_str_free(&whitelist_dcs);
      throw_invalid_argument(dcs, "dcs", "a string");

      return;
    }

    if (i > 0) {
      smart_str_appendl(&whitelist_dcs, ",", 1);
    }

    smart_str_appendl(&whitelist_dcs, Z_STRVAL_P(dcs), Z_STRLEN_P(dcs));
  }

  smart_str_0(&whitelist_dcs);

  efree(self->whitelist_dcs);
  self->whitelist_dcs = estrndup(whitelist_dcs.s->val, whitelist_dcs.s->len);
  smart_str_free(&whitelist_dcs);

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withTokenAwareRouting)
{
  zend_bool enabled = 1;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &enabled) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  self->use_token_aware_routing = enabled;

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withCredentials)
{
  zval* username = NULL;
  zval* password = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "zz", &username, &password) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(username) != IS_STRING) {
    INVALID_ARGUMENT(username, "a string");
  }

  if (Z_TYPE_P(password) != IS_STRING) {
    INVALID_ARGUMENT(password, "a string");
  }

  if (self->username) {
    efree(self->username);
    efree(self->password);
  }

  self->username = estrndup(Z_STRVAL_P(username), Z_STRLEN_P(username));
  self->password = estrndup(Z_STRVAL_P(password), Z_STRLEN_P(password));

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withConnectTimeout)
{
  zval* timeout = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &timeout) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(timeout) == IS_LONG && Z_LVAL_P(timeout) > 0) {
    self->connect_timeout = Z_LVAL_P(timeout) * 1000;
  } else if (Z_TYPE_P(timeout) == IS_DOUBLE && Z_DVAL_P(timeout) > 0) {
    self->connect_timeout = ceil(Z_DVAL_P(timeout) * 1000);
  } else {
    INVALID_ARGUMENT(timeout, "a positive number");
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withRequestTimeout)
{
  double timeout;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "d", &timeout) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  self->request_timeout = ceil(timeout * 1000);

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withSSL)
{
  zval* ssl_options = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &ssl_options, php_driver_ssl_ce) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (!PHP5TO7_ZVAL_IS_UNDEF(self->ssl_options))
    zval_ptr_dtor(&self->ssl_options);

  PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(self->ssl_options), ssl_options);

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withPersistentSessions)
{
  zend_bool enabled = 1;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &enabled) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  self->persist = enabled;

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withProtocolVersion)
{
  zval* version = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &version) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(version) == IS_LONG && Z_LVAL_P(version) >= 1) {
    self->protocol_version = Z_LVAL_P(version);
  } else {
    INVALID_ARGUMENT(version, "must be >= 1");
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withIOThreads)
{
  zval* count = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &count) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(count) == IS_LONG && Z_LVAL_P(count) < 129 && Z_LVAL_P(count) > 0) {
    self->io_threads = Z_LVAL_P(count);
  } else {
    INVALID_ARGUMENT(count, "a number between 1 and 128");
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withConnectionsPerHost)
{
  zval* core = NULL;
  zval* max  = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z", &core, &max) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(core) == IS_LONG && Z_LVAL_P(core) < 129 && Z_LVAL_P(core) > 0) {
    self->core_connections_per_host = Z_LVAL_P(core);
  } else {
    INVALID_ARGUMENT(core, "a number between 1 and 128");
  }

  if (max == NULL || Z_TYPE_P(max) == IS_NULL) {
    self->max_connections_per_host = Z_LVAL_P(core);
  } else if (Z_TYPE_P(max) == IS_LONG) {
    if (Z_LVAL_P(max) < Z_LVAL_P(core)) {
      INVALID_ARGUMENT(max, "greater than core");
    } else if (Z_LVAL_P(max) > 128) {
      INVALID_ARGUMENT(max, "less than 128");
    } else {
      self->max_connections_per_host = Z_LVAL_P(max);
    }
  } else {
    INVALID_ARGUMENT(max, "a number between 1 and 128");
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withReconnectInterval)
{
  zval* interval = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &interval) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(interval) == IS_LONG && Z_LVAL_P(interval) > 0) {
    self->reconnect_interval = Z_LVAL_P(interval) * 1000;
  } else if (Z_TYPE_P(interval) == IS_DOUBLE && Z_DVAL_P(interval) > 0) {
    self->reconnect_interval = ceil(Z_DVAL_P(interval) * 1000);
  } else {
    INVALID_ARGUMENT(interval, "a positive number");
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withLatencyAwareRouting)
{
  zend_bool enabled = 1;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &enabled) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  self->enable_latency_aware_routing = enabled;

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withTCPNodelay)
{
  zend_bool enabled = 1;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &enabled) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  self->enable_tcp_nodelay = enabled;

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withTCPKeepalive)
{
  zval* delay = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &delay) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(delay) == IS_NULL) {
    self->enable_tcp_keepalive = 0;
    self->tcp_keepalive_delay  = 0;
  } else if (Z_TYPE_P(delay) == IS_LONG && Z_LVAL_P(delay) > 0) {
    self->enable_tcp_keepalive = 1;
    self->tcp_keepalive_delay  = Z_LVAL_P(delay) * 1000;
  } else if (Z_TYPE_P(delay) == IS_DOUBLE && Z_DVAL_P(delay) > 0) {
    self->enable_tcp_keepalive = 1;
    self->tcp_keepalive_delay  = ceil(Z_DVAL_P(delay) * 1000);
  } else {
    INVALID_ARGUMENT(delay, "a positive number or null");
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withRetryPolicy)
{
  zval* retry_policy = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "O",
                            &retry_policy, php_driver_retry_policy_ce)
      == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (!PHP5TO7_ZVAL_IS_UNDEF(self->retry_policy))
    zval_ptr_dtor(&self->retry_policy);

  PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(self->retry_policy), retry_policy);

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withTimestampGenerator)
{
  zval* timestamp_gen = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "O",
                            &timestamp_gen, php_driver_timestamp_gen_ce)
      == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (!PHP5TO7_ZVAL_IS_UNDEF(self->timestamp_gen))
    zval_ptr_dtor(&self->timestamp_gen);

  PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(self->timestamp_gen), timestamp_gen);

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withSchemaMetadata)
{
  zend_bool enabled = 1;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &enabled) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  self->enable_schema = enabled;

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withHostnameResolution)
{
  zend_bool enabled = 1;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &enabled) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  self->enable_hostname_resolution = enabled;

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withRandomizedContactPoints)
{
  zend_bool enabled = 1;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &enabled) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  self->enable_randomized_contact_points = enabled;

  RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_METHOD(Cassandra_Cluster_Builder, withConnectionHeartbeatInterval)
{
  zval* interval = NULL;
  php_driver_cluster_builder* self;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &interval) == FAILURE) {
    return;
  }

  self = PHP_DRIVER_CLUSTER_BUILDER_THIS();

  if (Z_TYPE_P(interval) == IS_LONG && Z_LVAL_P(interval) >= 0) {
    self->connection_heartbeat_interval = Z_LVAL_P(interval);
  } else if (Z_TYPE_P(interval) == IS_DOUBLE && Z_DVAL_P(interval) >= 0) {
    self->connection_heartbeat_interval = ceil(Z_DVAL_P(interval));
  } else {
    INVALID_ARGUMENT(interval, "a positive number (or 0 to disable)");
  }

  RETURN_ZVAL(getThis(), 1, 0);
}

static zend_object_handlers php_driver_cluster_builder_handlers;

static HashTable*
php_driver_cluster_builder_gc(
#if PHP_MAJOR_VERSION >= 8
  zend_object* object,
#else
  zval* object,
#endif
  zval** table,
  int* n)
{
  *table = NULL;
  *n     = 0;
  return zend_std_get_properties(object);
}

static HashTable*
php_driver_cluster_builder_properties(
#if PHP_MAJOR_VERSION >= 8
  zend_object* object
#else
  zval* object
#endif
)
{
  zval contactPoints;
  zval loadBalancingPolicy;
  zval localDatacenter;
  zval hostPerRemoteDatacenter;
  zval useRemoteDatacenterForLocalConsistencies;
  zval useTokenAwareRouting;
  zval username;
  zval password;
  zval connectTimeout;
  zval requestTimeout;
  zval sslOptions;
  zval defaultConsistency;
  zval defaultPageSize;
  zval defaultTimeout;
  zval usePersistentSessions;
  zval protocolVersion;
  zval ioThreads;
  zval coreConnectionPerHost;
  zval maxConnectionsPerHost;
  zval reconnectInterval;
  zval latencyAwareRouting;
  zval tcpNodelay;
  zval tcpKeepalive;
  zval retryPolicy;
  zval blacklistHosts;
  zval whitelistHosts;
  zval blacklistDCs;
  zval whitelistDCs;
  zval timestampGen;
  zval schemaMetadata;
  zval hostnameResolution;
  zval randomizedContactPoints;
  zval connectionHeartbeatInterval;

  php_driver_cluster_builder* self = PHP_DRIVER_CLUSTER_BUILDER_OBJECT(object);
  HashTable* props                 = zend_std_get_properties(object);


  ZVAL_STRING(PHP5TO7_ZVAL_MAYBE_P(contactPoints), self->contact_points);


  ZVAL_LONG(PHP5TO7_ZVAL_MAYBE_P(loadBalancingPolicy), self->load_balancing_policy);




  if (self->load_balancing_policy == LOAD_BALANCING_DC_AWARE_ROUND_ROBIN) {
    ZVAL_STRING(PHP5TO7_ZVAL_MAYBE_P(localDatacenter), self->local_dc);
    ZVAL_LONG(PHP5TO7_ZVAL_MAYBE_P(hostPerRemoteDatacenter), self->used_hosts_per_remote_dc);
    ZVAL_BOOL(PHP5TO7_ZVAL_MAYBE_P(useRemoteDatacenterForLocalConsistencies), self->allow_remote_dcs_for_local_cl);
  } else {
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(localDatacenter));
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(hostPerRemoteDatacenter));
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(useRemoteDatacenterForLocalConsistencies));
  }


  ZVAL_BOOL(PHP5TO7_ZVAL_MAYBE_P(useTokenAwareRouting), self->use_token_aware_routing);



  if (self->username) {
    ZVAL_STRING(PHP5TO7_ZVAL_MAYBE_P(username), self->username);
    ZVAL_STRING(PHP5TO7_ZVAL_MAYBE_P(password), self->password);
  } else {
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(username));
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(password));
  }


  ZVAL_DOUBLE(PHP5TO7_ZVAL_MAYBE_P(connectTimeout), (double) self->connect_timeout / 1000);

  ZVAL_DOUBLE(PHP5TO7_ZVAL_MAYBE_P(requestTimeout), (double) self->request_timeout / 1000);


  if (!PHP5TO7_ZVAL_IS_UNDEF(self->ssl_options)) {
    PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(sslOptions), PHP5TO7_ZVAL_MAYBE_P(self->ssl_options));
  } else {
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(sslOptions));
  }


  ZVAL_LONG(PHP5TO7_ZVAL_MAYBE_P(defaultConsistency), self->default_consistency);

  ZVAL_LONG(PHP5TO7_ZVAL_MAYBE_P(defaultPageSize), self->default_page_size);

  if (!PHP5TO7_ZVAL_IS_UNDEF(self->default_timeout)) {
    ZVAL_LONG(PHP5TO7_ZVAL_MAYBE_P(defaultTimeout), PHP5TO7_Z_LVAL_MAYBE_P(self->default_timeout));
  } else {
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(defaultTimeout));
  }


  ZVAL_BOOL(PHP5TO7_ZVAL_MAYBE_P(usePersistentSessions), self->persist);


  ZVAL_LONG(PHP5TO7_ZVAL_MAYBE_P(protocolVersion), self->protocol_version);


  ZVAL_LONG(PHP5TO7_ZVAL_MAYBE_P(ioThreads), self->io_threads);


  ZVAL_LONG(PHP5TO7_ZVAL_MAYBE_P(coreConnectionPerHost), self->core_connections_per_host);


  ZVAL_LONG(PHP5TO7_ZVAL_MAYBE_P(maxConnectionsPerHost), self->max_connections_per_host);


  ZVAL_DOUBLE(PHP5TO7_ZVAL_MAYBE_P(reconnectInterval), (double) self->reconnect_interval / 1000);


  ZVAL_BOOL(PHP5TO7_ZVAL_MAYBE_P(latencyAwareRouting), self->enable_latency_aware_routing);


  ZVAL_BOOL(PHP5TO7_ZVAL_MAYBE_P(tcpNodelay), self->enable_tcp_nodelay);


  if (self->enable_tcp_keepalive) {
    ZVAL_DOUBLE(PHP5TO7_ZVAL_MAYBE_P(tcpKeepalive), (double) self->tcp_keepalive_delay / 1000);
  } else {
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(tcpKeepalive));
  }


  if (!PHP5TO7_ZVAL_IS_UNDEF(self->retry_policy)) {
    PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(retryPolicy), PHP5TO7_ZVAL_MAYBE_P(self->retry_policy));
  } else {
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(retryPolicy));
  }


  if (self->blacklist_hosts) {
    ZVAL_STRING(PHP5TO7_ZVAL_MAYBE_P(blacklistHosts), self->blacklist_hosts);
  } else {
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(blacklistHosts));
  }


  if (self->whitelist_hosts) {
    ZVAL_STRING(PHP5TO7_ZVAL_MAYBE_P(whitelistHosts), self->whitelist_hosts);
  } else {
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(whitelistHosts));
  }


  if (self->blacklist_dcs) {
    ZVAL_STRING(PHP5TO7_ZVAL_MAYBE_P(blacklistDCs), self->blacklist_dcs);
  } else {
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(blacklistDCs));
  }


  if (self->whitelist_dcs) {
    ZVAL_STRING(PHP5TO7_ZVAL_MAYBE_P(whitelistDCs), self->whitelist_dcs);
  } else {
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(whitelistDCs));
  }


  if (!PHP5TO7_ZVAL_IS_UNDEF(self->timestamp_gen)) {
    PHP5TO7_ZVAL_COPY(PHP5TO7_ZVAL_MAYBE_P(timestampGen), PHP5TO7_ZVAL_MAYBE_P(self->timestamp_gen));
  } else {
    ZVAL_NULL(PHP5TO7_ZVAL_MAYBE_P(timestampGen));
  }


  ZVAL_BOOL(PHP5TO7_ZVAL_MAYBE_P(schemaMetadata), self->enable_schema);


  ZVAL_BOOL(PHP5TO7_ZVAL_MAYBE_P(hostnameResolution), self->enable_hostname_resolution);


  ZVAL_BOOL(PHP5TO7_ZVAL_MAYBE_P(randomizedContactPoints), self->enable_randomized_contact_points);


  ZVAL_LONG(PHP5TO7_ZVAL_MAYBE_P(connectionHeartbeatInterval), self->connection_heartbeat_interval);

  PHP5TO7_ZEND_HASH_UPDATE(props, "contactPoints", sizeof("contactPoints"),
                           PHP5TO7_ZVAL_MAYBE_P(contactPoints), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "loadBalancingPolicy", sizeof("loadBalancingPolicy"),
                           PHP5TO7_ZVAL_MAYBE_P(loadBalancingPolicy), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "localDatacenter", sizeof("localDatacenter"),
                           PHP5TO7_ZVAL_MAYBE_P(localDatacenter), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "hostPerRemoteDatacenter", sizeof("hostPerRemoteDatacenter"),
                           PHP5TO7_ZVAL_MAYBE_P(hostPerRemoteDatacenter), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props,
                           "useRemoteDatacenterForLocalConsistencies",
                           sizeof("useRemoteDatacenterForLocalConsistencies"),
                           PHP5TO7_ZVAL_MAYBE_P(useRemoteDatacenterForLocalConsistencies),
                           sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "useTokenAwareRouting", sizeof("useTokenAwareRouting"),
                           PHP5TO7_ZVAL_MAYBE_P(useTokenAwareRouting), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "username", sizeof("username"),
                           PHP5TO7_ZVAL_MAYBE_P(username), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "password", sizeof("password"),
                           PHP5TO7_ZVAL_MAYBE_P(password), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "connectTimeout", sizeof("connectTimeout"),
                           PHP5TO7_ZVAL_MAYBE_P(connectTimeout), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "requestTimeout", sizeof("requestTimeout"),
                           PHP5TO7_ZVAL_MAYBE_P(requestTimeout), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "sslOptions", sizeof("sslOptions"),
                           PHP5TO7_ZVAL_MAYBE_P(sslOptions), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "defaultConsistency", sizeof("defaultConsistency"),
                           PHP5TO7_ZVAL_MAYBE_P(defaultConsistency), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "defaultPageSize", sizeof("defaultPageSize"),
                           PHP5TO7_ZVAL_MAYBE_P(defaultPageSize), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "defaultTimeout", sizeof("defaultTimeout"),
                           PHP5TO7_ZVAL_MAYBE_P(defaultTimeout), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "usePersistentSessions", sizeof("usePersistentSessions"),
                           PHP5TO7_ZVAL_MAYBE_P(usePersistentSessions), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "protocolVersion", sizeof("protocolVersion"),
                           PHP5TO7_ZVAL_MAYBE_P(protocolVersion), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "ioThreads", sizeof("ioThreads"),
                           PHP5TO7_ZVAL_MAYBE_P(ioThreads), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "coreConnectionPerHost", sizeof("coreConnectionPerHost"),
                           PHP5TO7_ZVAL_MAYBE_P(coreConnectionPerHost), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "maxConnectionsPerHost", sizeof("maxConnectionsPerHost"),
                           PHP5TO7_ZVAL_MAYBE_P(maxConnectionsPerHost), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "reconnectInterval", sizeof("reconnectInterval"),
                           PHP5TO7_ZVAL_MAYBE_P(reconnectInterval), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "latencyAwareRouting", sizeof("latencyAwareRouting"),
                           PHP5TO7_ZVAL_MAYBE_P(latencyAwareRouting), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "tcpNodelay", sizeof("tcpNodelay"),
                           PHP5TO7_ZVAL_MAYBE_P(tcpNodelay), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "tcpKeepalive", sizeof("tcpKeepalive"),
                           PHP5TO7_ZVAL_MAYBE_P(tcpKeepalive), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "retryPolicy", sizeof("retryPolicy"),
                           PHP5TO7_ZVAL_MAYBE_P(retryPolicy), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "timestampGenerator", sizeof("timestampGenerator"),
                           PHP5TO7_ZVAL_MAYBE_P(timestampGen), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "schemaMetadata", sizeof("schemaMetadata"),
                           PHP5TO7_ZVAL_MAYBE_P(schemaMetadata), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "blacklist_hosts", sizeof("blacklist_hosts"),
                           PHP5TO7_ZVAL_MAYBE_P(blacklistHosts), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "whitelist_hosts", sizeof("whitelist_hosts"),
                           PHP5TO7_ZVAL_MAYBE_P(whitelistHosts), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "blacklist_dcs", sizeof("blacklist_dcs"),
                           PHP5TO7_ZVAL_MAYBE_P(blacklistDCs), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "whitelist_dcs", sizeof("whitelist_dcs"),
                           PHP5TO7_ZVAL_MAYBE_P(whitelistDCs), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "hostnameResolution", sizeof("hostnameResolution"),
                           PHP5TO7_ZVAL_MAYBE_P(hostnameResolution), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "randomizedContactPoints", sizeof("randomizedContactPoints"),
                           PHP5TO7_ZVAL_MAYBE_P(randomizedContactPoints), sizeof(zval));
  PHP5TO7_ZEND_HASH_UPDATE(props, "connectionHeartbeatInterval", sizeof("connectionHeartbeatInterval"),
                           PHP5TO7_ZVAL_MAYBE_P(connectionHeartbeatInterval), sizeof(zval));

  return props;
}

static int
php_driver_cluster_builder_compare(zval* obj1, zval* obj2)
{
  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
    return 1; /* different classes */

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void
php_driver_cluster_builder_free(zend_object* object)
{
  php_driver_cluster_builder* self = PHP_DRIVER_CLUSTER_BUILDER_OBJECT(object);

  efree(self->contact_points);
  self->contact_points = NULL;

  if (self->local_dc) {
    efree(self->local_dc);
    self->local_dc = NULL;
  }

  if (self->username) {
    efree(self->username);
    self->username = NULL;
  }

  if (self->password) {
    efree(self->password);
    self->password = NULL;
  }

  if (self->whitelist_hosts) {
    efree(self->whitelist_hosts);
    self->whitelist_hosts = NULL;
  }

  if (self->blacklist_hosts) {
    efree(self->blacklist_hosts);
    self->blacklist_hosts = NULL;
  }

  if (self->whitelist_dcs) {
    efree(self->whitelist_dcs);
    self->whitelist_dcs = NULL;
  }

  if (self->blacklist_dcs) {
    efree(self->blacklist_dcs);
    self->whitelist_dcs = NULL;
  }

  ZVAL_DESTROY(self->ssl_options);
  ZVAL_DESTROY(self->default_timeout);
  ZVAL_DESTROY(self->retry_policy);
  ZVAL_DESTROY(self->timestamp_gen);

  zend_object_std_dtor(&self->zval);
}

static zend_object*
php_driver_cluster_builder_new(zend_class_entry* ce)
{
  php_driver_cluster_builder* self = make(php_driver_cluster_builder);

  self->contact_points                   = estrdup("127.0.0.1");
  self->port                             = 9042;
  self->load_balancing_policy            = LOAD_BALANCING_DEFAULT;
  self->local_dc                         = NULL;
  self->used_hosts_per_remote_dc         = 0;
  self->allow_remote_dcs_for_local_cl    = 0;
  self->use_token_aware_routing          = 1;
  self->username                         = NULL;
  self->password                         = NULL;
  self->connect_timeout                  = 5000;
  self->request_timeout                  = 12000;
  self->default_consistency              = PHP_DRIVER_DEFAULT_CONSISTENCY;
  self->default_page_size                = 5000;
  self->persist                          = 1;
  self->protocol_version                 = 4;
  self->io_threads                       = 1;
  self->core_connections_per_host        = 1;
  self->max_connections_per_host         = 2;
  self->reconnect_interval               = 2000;
  self->enable_latency_aware_routing     = 1;
  self->enable_tcp_nodelay               = 1;
  self->enable_tcp_keepalive             = 0;
  self->tcp_keepalive_delay              = 0;
  self->enable_schema                    = 1;
  self->blacklist_hosts                  = NULL;
  self->whitelist_hosts                  = NULL;
  self->blacklist_dcs                    = NULL;
  self->whitelist_dcs                    = NULL;
  self->enable_hostname_resolution       = 0;
  self->enable_randomized_contact_points = 1;
  self->connection_heartbeat_interval    = 30;

  ZVAL_UNDEF(&self->ssl_options);
  ZVAL_UNDEF(&self->default_timeout);
  ZVAL_UNDEF(&self->retry_policy);
  ZVAL_UNDEF(&self->timestamp_gen);

  PHP5TO7_ZEND_OBJECT_INIT(cluster_builder, self, ce);
}

void
php_driver_define_ClusterBuilder()
{
  php_driver_cluster_builder_ce                = register_class_Cassandra_Cluster_Builder();
  php_driver_cluster_builder_ce->create_object = php_driver_cluster_builder_new;

  memcpy(&php_driver_cluster_builder_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_cluster_builder_handlers.get_properties = php_driver_cluster_builder_properties;
  php_driver_cluster_builder_handlers.get_gc         = php_driver_cluster_builder_gc;
  php_driver_cluster_builder_handlers.compare        = php_driver_cluster_builder_compare;
}
