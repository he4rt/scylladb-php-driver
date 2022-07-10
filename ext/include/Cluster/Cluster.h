#pragma once

#include <cassandra.h>
#include <php.h>

#include <CassandraDriver.h>

typedef struct {
  cass_byte_t* data;
  CassCluster* cluster;
  long default_consistency;
  int default_page_size;
  zval default_timeout;
  cass_bool_t persist;
  char* hash_key;
  int hash_key_len;
  zend_object zval;
} PhpDriverCluster;

typedef enum {
  LOAD_BALANCING_DEFAULT = 0,
  LOAD_BALANCING_ROUND_ROBIN,
  LOAD_BALANCING_DC_AWARE_ROUND_ROBIN
} PhpDriverLoadBalancing;

typedef struct {
  char* contact_points;
  int port;
  PhpDriverLoadBalancing load_balancing_policy;
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
} PhpDriverClusterBuilder;

#define PHP_DRIVER_CLUSTER_OBJECT(obj) PHP_DRIVER_OBJECT(PhpDriverCluster, obj)
#define PHP_DRIVER_CLUSTER_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(PhpDriverCluster, obj)
#define PHP_DRIVER_CLUSTER_THIS() PHP_DRIVER_THIS(PhpDriverCluster)

#define PHP_DRIVER_CLUSTER_BUILDER_OBJECT(obj) PHP_DRIVER_OBJECT(PhpDriverClusterBuilder, obj)
#define PHP_DRIVER_CLUSTER_BUILDER_ZVAL_TO_OBJECT(obj) PHP_DRIVER_ZVAL_TO_OBJECT(PhpDriverClusterBuilder, obj)
#define PHP_DRIVER_CLUSTER_BUILDER_THIS() PHP_DRIVER_THIS(PhpDriverClusterBuilder)

extern PHP_DRIVER_API zend_class_entry* phpDriverClusterInterfaceCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverDefaultClusterCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverClusterBuilderCe;

void PhpDriverDefineCluster();