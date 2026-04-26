#include <php.h>

#include <php_driver.h>
#include <php_driver_types.h>

#include "BuilderHandlers.h"

BEGIN_EXTERN_C()
static zend_object_handlers php_driver_cluster_builder_handlers;

static HashTable *php_driver_cluster_builder_gc(zend_object *object, zval **table, int *n)
{
    *table = nullptr;
    *n = 0;
    return NULL;
}
static HashTable *php_driver_cluster_builder_properties(zend_object *object)
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

    php_driver_cluster_builder *self = php_driver_cluster_builder_object_fetch(object);
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
        ZVAL_STR_COPY(&password, self->password);
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

    (void)zend_hash_str_update(props, "contactPoints", sizeof("contactPoints") - 1, &contactPoints);
    (void)zend_hash_str_update(props, "loadBalancingPolicy", sizeof("loadBalancingPolicy") - 1, &loadBalancingPolicy);
    (void)zend_hash_str_update(props, "localDatacenter", sizeof("localDatacenter") - 1, &localDatacenter);
    (void)zend_hash_str_update(props, "hostPerRemoteDatacenter", sizeof("hostPerRemoteDatacenter") - 1, &hostPerRemoteDatacenter);
    (void)zend_hash_str_update(props, "useRemoteDatacenterForLocalConsistencies", sizeof("useRemoteDatacenterForLocalConsistencies") - 1, &useRemoteDatacenterForLocalConsistencies);
    (void)zend_hash_str_update(props, "useTokenAwareRouting", sizeof("useTokenAwareRouting") - 1, &useTokenAwareRouting);
    (void)zend_hash_str_update(props, "username", sizeof("username") - 1, &username);
    (void)zend_hash_str_update(props, "password", sizeof("password") - 1, &password);
    (void)zend_hash_str_update(props, "connectTimeout", sizeof("connectTimeout") - 1, &connectTimeout);
    (void)zend_hash_str_update(props, "requestTimeout", sizeof("requestTimeout") - 1, &requestTimeout);
    (void)zend_hash_str_update(props, "sslOptions", sizeof("sslOptions") - 1, &sslOptions);
    (void)zend_hash_str_update(props, "defaultConsistency", sizeof("defaultConsistency") - 1, &defaultConsistency);
    (void)zend_hash_str_update(props, "defaultPageSize", sizeof("defaultPageSize") - 1, &defaultPageSize);
    (void)zend_hash_str_update(props, "defaultTimeout", sizeof("defaultTimeout") - 1, &defaultTimeout);
    (void)zend_hash_str_update(props, "usePersistentSessions", sizeof("usePersistentSessions") - 1, &usePersistentSessions);
    (void)zend_hash_str_update(props, "protocolVersion", sizeof("protocolVersion") - 1, &protocolVersion);
    (void)zend_hash_str_update(props, "ioThreads", sizeof("ioThreads") - 1, &ioThreads);
    (void)zend_hash_str_update(props, "coreConnectionPerHost", sizeof("coreConnectionPerHost") - 1, &coreConnectionPerHost);
    (void)zend_hash_str_update(props, "maxConnectionsPerHost", sizeof("maxConnectionsPerHost") - 1, &maxConnectionsPerHost);
    (void)zend_hash_str_update(props, "reconnectInterval", sizeof("reconnectInterval") - 1, &reconnectInterval);
    (void)zend_hash_str_update(props, "latencyAwareRouting", sizeof("latencyAwareRouting") - 1, &latencyAwareRouting);
    (void)zend_hash_str_update(props, "tcpNodelay", sizeof("tcpNodelay") - 1, &tcpNodelay);
    (void)zend_hash_str_update(props, "tcpKeepalive", sizeof("tcpKeepalive") - 1, &tcpKeepalive);
    (void)zend_hash_str_update(props, "retryPolicy", sizeof("retryPolicy") - 1, &retryPolicy);
    (void)zend_hash_str_update(props, "timestampGenerator", sizeof("timestampGenerator") - 1, &timestampGen);
    (void)zend_hash_str_update(props, "schemaMetadata", sizeof("schemaMetadata") - 1, &schemaMetadata);
    (void)zend_hash_str_update(props, "blacklist_hosts", sizeof("blacklist_hosts") - 1, &blacklistHosts);
    (void)zend_hash_str_update(props, "whitelist_hosts", sizeof("whitelist_hosts") - 1, &whitelistHosts);
    (void)zend_hash_str_update(props, "blacklist_dcs", sizeof("blacklist_dcs") - 1, &blacklistDCs);
    (void)zend_hash_str_update(props, "whitelist_dcs", sizeof("whitelist_dcs") - 1, &whitelistDCs);
    (void)zend_hash_str_update(props, "hostnameResolution", sizeof("hostnameResolution") - 1, &hostnameResolution);
    (void)zend_hash_str_update(props, "randomizedContactPoints", sizeof("randomizedContactPoints") - 1, &randomizedContactPoints);
    (void)zend_hash_str_update(props, "connectionHeartbeatInterval", sizeof("connectionHeartbeatInterval") - 1, &connectionHeartbeatInterval);

    return props;
}
static int php_driver_cluster_builder_compare(zval *obj1, zval *obj2)
{
    if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2))
        return 1; /* different classes */

    return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj2);
}
static void php_driver_cluster_builder_free(zend_object *object)
{
    php_driver_cluster_builder *self = PHP5TO7_ZEND_OBJECT_GET(cluster_builder, object);

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
        self->whitelist_dcs = nullptr;
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
zend_object* php_driver_cluster_builder_new(zend_class_entry *ce)
{
    auto *self = static_cast<php_driver_cluster_builder *>(
        emalloc(sizeof(php_driver_cluster_builder) + zend_object_properties_size(ce)));

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

    zend_object_std_init(&self->zendObject, ce);
    self->zendObject.handlers = &php_driver_cluster_builder_handlers;

    if (zend_object_properties_size(ce) > 0)
    {
        object_properties_init(&self->zendObject, ce);
    }

    return &self->zendObject;
}

END_EXTERN_C()

void php_driver_initialize_cluster_builder_handlers()
{
    memcpy(&php_driver_cluster_builder_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    php_driver_cluster_builder_handlers.get_properties = php_driver_cluster_builder_properties;
    php_driver_cluster_builder_handlers.get_gc = php_driver_cluster_builder_gc;
    php_driver_cluster_builder_handlers.compare = php_driver_cluster_builder_compare;
    php_driver_cluster_builder_handlers.offset = XtOffsetOf(php_driver_cluster_builder, zendObject);
    php_driver_cluster_builder_handlers.free_obj = php_driver_cluster_builder_free;
}
