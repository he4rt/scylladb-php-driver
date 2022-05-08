//
// Created by Dusan Malusev on 5/6/22.
//

#ifndef LIBPHPCASSANDRA_EXT_INCLUDE_CLUSTER_CLUSTER_H_
#define LIBPHPCASSANDRA_EXT_INCLUDE_CLUSTER_CLUSTER_H_

#include "php_driver.h"

#include <cassandra_driver.h>

typedef struct php_driver_cluster_ {
  cass_byte_t* data;
  CassCluster* cluster;
  long default_consistency;
  int default_page_size;
  zval default_timeout;
  cass_bool_t persist;
  char* hash_key;
  int hash_key_len;
  zend_object zval;
} php_driver_cluster;

typedef enum {
  LOAD_BALANCING_DEFAULT = 0,
  LOAD_BALANCING_ROUND_ROBIN,
  LOAD_BALANCING_DC_AWARE_ROUND_ROBIN
} php_driver_load_balancing;

typedef struct php_driver_cluster_builder_ {
  char* contact_points;
  int port;
  php_driver_load_balancing load_balancing_policy;
  char* local_dc;
  unsigned int used_hosts_per_remote_dc;
  cass_bool_t allow_remote_dcs_for_local_cl;
  cass_bool_t use_token_aware_routing;
  char* username;
  char* password;
  unsigned int connect_timeout;
  unsigned int request_timeout;
  zval ssl_options;
  long default_consistency;
  int default_page_size;
  zval default_timeout;
  cass_bool_t persist;
  int protocol_version;
  int io_threads;
  int core_connections_per_host;
  int max_connections_per_host;
  unsigned int reconnect_interval;
  cass_bool_t enable_latency_aware_routing;
  cass_bool_t enable_tcp_nodelay;
  cass_bool_t enable_tcp_keepalive;
  unsigned int tcp_keepalive_delay;
  zval retry_policy;
  zval timestamp_gen;
  cass_bool_t enable_schema;
  char* blacklist_hosts;
  char* whitelist_hosts;
  char* blacklist_dcs;
  char* whitelist_dcs;
  cass_bool_t enable_hostname_resolution;
  cass_bool_t enable_randomized_contact_points;
  unsigned int connection_heartbeat_interval;
  zend_object zval;
} php_driver_cluster_builder;

#define PHP_DRIVER_CLUSTER_OBJECT(obj) PHP_DRIVER_OBJECT(php_driver_cluster, obj)
#define PHP_DRIVER_CLUSTER_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(php_driver_cluster, obj)
#define PHP_DRIVER_CLUSTER_THIS() PHP_DRIVER_THIS(php_driver_cluster)

#define PHP_DRIVER_CLUSTER_BUILDER_OBJECT(obj) PHP_DRIVER_OBJECT(php_driver_cluster_builder, obj)
#define PHP_DRIVER_CLUSTER_BUILDER_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(php_driver_cluster_builder, obj)
#define PHP_DRIVER_CLUSTER_BUILDER_THIS() PHP_DRIVER_THIS(php_driver_cluster_builder)

extern PHP_DRIVER_API zend_class_entry* php_driver_cluster_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_default_cluster_ce;
extern PHP_DRIVER_API zend_class_entry* php_driver_cluster_builder_ce;
#endif // LIBPHPCASSANDRA_EXT_INCLUDE_CLUSTER_CLUSTER_H_
