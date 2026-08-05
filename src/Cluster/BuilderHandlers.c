#include <php.h>

#include <php_scylladb.h>
#include <php_scylladb_types.h>

#include "BuilderHandlers.h"

extern zend_object_handlers php_scylladb_cluster_builder_handlers;

HashTable *php_scylladb_cluster_builder_gc(zend_object *object, zval **table, int *n)
{
    /* Expose the only zval the builder holds (default_timeout) via the GC
     * buffer. Going through zend_std_get_gc would instead invoke the
     * side-effecting get_properties (it rebuilds object->properties on every
     * call), which corrupts refcounts across the collector's mark/scan passes. */
    auto self = php_scylladb_cluster_builder_object_fetch(object);
    zend_get_gc_buffer *buffer = zend_get_gc_buffer_create();
    zend_get_gc_buffer_add_zval(buffer, &self->default_timeout);
    zend_get_gc_buffer_use(buffer, table, n);
    return nullptr;
}
HashTable *php_scylladb_cluster_builder_properties(zend_object *object)
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

    auto self = php_scylladb_cluster_builder_object_fetch(object);
    if (object->properties) {
        zend_array_release(object->properties);
    }
    object->properties = zend_new_array(32);
    HashTable *props = object->properties;

    ZVAL_STR_COPY(&contactPoints, self->contact_points);

    ZVAL_LONG(&loadBalancingPolicy, self->load_balancing_policy);

    if (self->load_balancing_policy == LOAD_BALANCING_DC_AWARE_ROUND_ROBIN)
    {
        ZVAL_STR_COPY(&localDatacenter, self->local_dc);
        ZVAL_LONG(&hostPerRemoteDatacenter, self->used_hosts_per_remote_dc);
        ZVAL_BOOL(&useRemoteDatacenterForLocalConsistencies, self->allow_remote_dcs_for_local_cl);
    }
    else
    {
        ZVAL_NULL(&localDatacenter);
        ZVAL_NULL(&hostPerRemoteDatacenter);
        ZVAL_NULL(&useRemoteDatacenterForLocalConsistencies);
    }

    ZVAL_BOOL(&useTokenAwareRouting, self->use_token_aware_routing);

    if (self->username)
    {
        ZVAL_STR_COPY(&username, self->username);
        /* Never put the credential itself in the properties table: it would be
         * printed by var_dump/print_r/(array) and by every framework debug
         * renderer, defeating the #[\SensitiveParameter] on withCredentials(). */
        ZVAL_STRINGL(&password, "***", 3);
    }
    else
    {
        ZVAL_NULL(&username);
        ZVAL_NULL(&password);
    }

    ZVAL_DOUBLE(&connectTimeout, (double)self->connect_timeout / 1000);
    ZVAL_DOUBLE(&requestTimeout, (double)self->request_timeout / 1000);
    if (self->ssl_options != nullptr)
    {
        ZVAL_OBJ_COPY(&sslOptions, &self->ssl_options->zendObject);
    }
    else
    {
        ZVAL_NULL(&sslOptions);
    }

    ZVAL_LONG(&defaultConsistency, self->default_consistency);
    ZVAL_LONG(&defaultPageSize, self->default_page_size);
    if (!Z_ISUNDEF(self->default_timeout))
    {
        ZVAL_LONG(&defaultTimeout, Z_LVAL(self->default_timeout));
    }
    else
    {
        ZVAL_NULL(&defaultTimeout);
    }

    ZVAL_BOOL(&usePersistentSessions, self->persist);
    ZVAL_LONG(&protocolVersion, self->protocol_version);
    ZVAL_LONG(&ioThreads, self->io_threads);
    ZVAL_LONG(&coreConnectionPerHost, self->core_connections_per_host);
    ZVAL_LONG(&maxConnectionsPerHost, self->max_connections_per_host);
    ZVAL_DOUBLE(&reconnectInterval, (double)self->reconnect_interval / 1000);
    ZVAL_BOOL(&latencyAwareRouting, self->enable_latency_aware_routing);
    ZVAL_BOOL(&tcpNodelay, self->enable_tcp_nodelay);

    if (self->enable_tcp_keepalive)
    {
        ZVAL_DOUBLE(&tcpKeepalive, (double)self->tcp_keepalive_delay / 1000);
    }
    else
    {
        ZVAL_NULL(&tcpKeepalive);
    }

    if (self->retry_policy != nullptr)
    {
        ZVAL_OBJ_COPY(&retryPolicy, &self->retry_policy->zendObject);
    }
    else
    {
        ZVAL_NULL(&retryPolicy);
    }

    if (self->blacklist_hosts != nullptr)
    {
        ZVAL_STR_COPY(&blacklistHosts, self->blacklist_hosts);
    }
    else
    {
        ZVAL_NULL(&blacklistHosts);
    }

    if (self->whitelist_hosts)
    {
        ZVAL_STR_COPY(&whitelistHosts, self->whitelist_hosts);
    }
    else
    {
        ZVAL_NULL(&whitelistHosts);
    }

    if (self->blacklist_dcs)
    {
        ZVAL_STR_COPY(&blacklistDCs, self->blacklist_dcs);
    }
    else
    {
        ZVAL_NULL(&blacklistDCs);
    }

    if (self->whitelist_dcs)
    {
        ZVAL_STR_COPY(&whitelistDCs, self->whitelist_dcs);
    }
    else
    {
        ZVAL_NULL(&whitelistDCs);
    }

    if (self->timestamp_gen != nullptr)
    {
        ZVAL_OBJ_COPY(&timestampGen, &self->timestamp_gen->zendObject);
    }
    else
    {
        ZVAL_NULL(&timestampGen);
    }

    ZVAL_BOOL(&schemaMetadata, self->enable_schema);

    ZVAL_BOOL(&hostnameResolution, self->enable_hostname_resolution);

    ZVAL_BOOL(&randomizedContactPoints, self->enable_randomized_contact_points);

    ZVAL_LONG(&connectionHeartbeatInterval, self->connection_heartbeat_interval);

    (void)zend_hash_str_update(props, ZEND_STRL("contactPoints"), &contactPoints);
    (void)zend_hash_str_update(props, ZEND_STRL("loadBalancingPolicy"), &loadBalancingPolicy);
    (void)zend_hash_str_update(props, ZEND_STRL("localDatacenter"), &localDatacenter);
    (void)zend_hash_str_update(props, ZEND_STRL("hostPerRemoteDatacenter"), &hostPerRemoteDatacenter);
    (void)zend_hash_str_update(props, ZEND_STRL("useRemoteDatacenterForLocalConsistencies"), &useRemoteDatacenterForLocalConsistencies);
    (void)zend_hash_str_update(props, ZEND_STRL("useTokenAwareRouting"), &useTokenAwareRouting);
    (void)zend_hash_str_update(props, ZEND_STRL("username"), &username);
    (void)zend_hash_str_update(props, ZEND_STRL("password"), &password);
    (void)zend_hash_str_update(props, ZEND_STRL("connectTimeout"), &connectTimeout);
    (void)zend_hash_str_update(props, ZEND_STRL("requestTimeout"), &requestTimeout);
    (void)zend_hash_str_update(props, ZEND_STRL("sslOptions"), &sslOptions);
    (void)zend_hash_str_update(props, ZEND_STRL("defaultConsistency"), &defaultConsistency);
    (void)zend_hash_str_update(props, ZEND_STRL("defaultPageSize"), &defaultPageSize);
    (void)zend_hash_str_update(props, ZEND_STRL("defaultTimeout"), &defaultTimeout);
    (void)zend_hash_str_update(props, ZEND_STRL("usePersistentSessions"), &usePersistentSessions);
    (void)zend_hash_str_update(props, ZEND_STRL("protocolVersion"), &protocolVersion);
    (void)zend_hash_str_update(props, ZEND_STRL("ioThreads"), &ioThreads);
    (void)zend_hash_str_update(props, ZEND_STRL("coreConnectionPerHost"), &coreConnectionPerHost);
    (void)zend_hash_str_update(props, ZEND_STRL("maxConnectionsPerHost"), &maxConnectionsPerHost);
    (void)zend_hash_str_update(props, ZEND_STRL("reconnectInterval"), &reconnectInterval);
    (void)zend_hash_str_update(props, ZEND_STRL("latencyAwareRouting"), &latencyAwareRouting);
    (void)zend_hash_str_update(props, ZEND_STRL("tcpNodelay"), &tcpNodelay);
    (void)zend_hash_str_update(props, ZEND_STRL("tcpKeepalive"), &tcpKeepalive);
    (void)zend_hash_str_update(props, ZEND_STRL("retryPolicy"), &retryPolicy);
    (void)zend_hash_str_update(props, ZEND_STRL("timestampGenerator"), &timestampGen);
    (void)zend_hash_str_update(props, ZEND_STRL("schemaMetadata"), &schemaMetadata);
    (void)zend_hash_str_update(props, ZEND_STRL("blacklist_hosts"), &blacklistHosts);
    (void)zend_hash_str_update(props, ZEND_STRL("whitelist_hosts"), &whitelistHosts);
    (void)zend_hash_str_update(props, ZEND_STRL("blacklist_dcs"), &blacklistDCs);
    (void)zend_hash_str_update(props, ZEND_STRL("whitelist_dcs"), &whitelistDCs);
    (void)zend_hash_str_update(props, ZEND_STRL("hostnameResolution"), &hostnameResolution);
    (void)zend_hash_str_update(props, ZEND_STRL("randomizedContactPoints"), &randomizedContactPoints);
    (void)zend_hash_str_update(props, ZEND_STRL("connectionHeartbeatInterval"), &connectionHeartbeatInterval);

    return props;
}
int php_scylladb_cluster_builder_compare(zval *obj1, zval *obj2)
{
    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return strcmp(ZSTR_VAL(Z_OBJCE_P(obj1)->name), ZSTR_VAL(Z_OBJCE_P(obj2)->name)); /* different classes */

    return (Z_OBJ_HANDLE_P(obj1) < Z_OBJ_HANDLE_P(obj2)) ? -1 : (Z_OBJ_HANDLE_P(obj1) > Z_OBJ_HANDLE_P(obj2));
}
void php_scylladb_cluster_builder_free(zend_object *object)
{
    auto self = php_scylladb_cluster_builder_object_fetch(object);

    zend_string_release(self->contact_points);
    self->contact_points = nullptr;

    if (self->local_dc != nullptr)
    {
        zend_string_release(self->local_dc);
        self->local_dc = nullptr;
    }

    if (self->username)
    {
        zend_string_release(self->username);
        self->username = nullptr;
    }

    if (self->password)
    {
        zend_string_release(self->password);
        self->password = nullptr;
    }

    if (self->whitelist_hosts != nullptr)
    {
        zend_string_release(self->whitelist_hosts);
        self->whitelist_hosts = nullptr;
    }

    if (self->blacklist_hosts != nullptr)
    {
        zend_string_release(self->blacklist_hosts);
        self->blacklist_hosts = nullptr;
    }

    if (self->whitelist_dcs)
    {
        zend_string_release(self->whitelist_dcs);
        self->whitelist_dcs = nullptr;
    }

    if (self->blacklist_dcs)
    {
        zend_string_release(self->blacklist_dcs);
        self->blacklist_dcs = nullptr;
    }

    if (self->ssl_options != nullptr)
    {
        zend_object_release(&self->ssl_options->zendObject);
        self->ssl_options = nullptr;
    }

    if (!Z_ISUNDEF(self->default_timeout))
    {
        zval_ptr_dtor(&self->default_timeout);
        ZVAL_UNDEF(&self->default_timeout);
    }

    if (self->retry_policy != nullptr)
    {
        zend_object_release(&self->retry_policy->zendObject);
        self->retry_policy = nullptr;
    }

    if (self->timestamp_gen)
    {
        zend_object_release(&self->timestamp_gen->zendObject);
        self->timestamp_gen = nullptr;
    }

    zend_object_std_dtor(object);
}
zend_object *php_scylladb_cluster_builder_new(zend_class_entry *ce)
{
    php_scylladb_cluster_builder *self =
        PHP_SCYLLADB_OBJ_ALLOCATE(php_scylladb_cluster_builder, ce, &php_scylladb_cluster_builder_handlers);

    self->contact_points = zend_string_init_fast(ZEND_STRL("127.0.0.1"));
    self->port = 9042;
    self->load_balancing_policy = LOAD_BALANCING_DEFAULT;
    self->local_dc = nullptr;
    self->used_hosts_per_remote_dc = 0;
    self->allow_remote_dcs_for_local_cl = cass_false;
    self->use_token_aware_routing = cass_true;
    self->username = nullptr;
    self->password = nullptr;
    self->connect_timeout = 5000;
    self->request_timeout = 12000;
    self->default_consistency = CASS_CONSISTENCY_LOCAL_ONE;
    self->default_page_size = 5000;
    self->persist = cass_true;
    self->protocol_version = 4;
    self->io_threads = 1;
    self->core_connections_per_host = 1;
    self->max_connections_per_host = 2;
    self->reconnect_interval = 2000;
    self->enable_latency_aware_routing = cass_true;
    self->enable_tcp_nodelay = cass_true;
    self->enable_tcp_keepalive = cass_false;
    self->tcp_keepalive_delay = 0;
    self->enable_schema = cass_true;
    self->blacklist_hosts = nullptr;
    self->whitelist_hosts = nullptr;
    self->blacklist_dcs = nullptr;
    self->whitelist_dcs = nullptr;
    self->enable_hostname_resolution = cass_false;
    self->enable_randomized_contact_points = cass_true;
    self->connection_heartbeat_interval = 30;
    self->timestamp_gen = nullptr;
    self->retry_policy = nullptr;
    self->ssl_options = nullptr;

    ZVAL_UNDEF(&self->default_timeout);

    return &self->zendObject;
}
