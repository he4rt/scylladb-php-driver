#include <php.h>

#include <php_scylladb.h>
#include <php_scylladb_globals.h>
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

    /* The profile zvals are strong refs the collector must be able to see. */
    if (self->execution_profiles != nullptr)
    {
        zval *profile_val = nullptr;
        ZEND_HASH_FOREACH_VAL(self->execution_profiles, profile_val)
        {
            zend_get_gc_buffer_add_zval(buffer, profile_val);
        }
        ZEND_HASH_FOREACH_END();
    }

    zend_get_gc_buffer_use(buffer, table, n);
    return nullptr;
}
HashTable *php_scylladb_cluster_builder_properties(zend_object *object)
{
    zval contactPoints;
    zval loadBalancingPolicy;
    zval localDatacenter;
    zval localRack;
    zval applicationName;
    zval applicationVersion;
    zval reconnectPolicy;
    zval reconnectMaxInterval;
    zval speculativeExecutionDelay;
    zval speculativeExecutionMax;
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
        /* Rack-aware routing also carries a local datacenter, and both of its
         * strings may be empty: the driver then infers them from the first
         * contact point it reaches. */
        if (self->local_dc != nullptr)
        {
            ZVAL_STR_COPY(&localDatacenter, self->local_dc);
        }
        else
        {
            ZVAL_NULL(&localDatacenter);
        }
        ZVAL_NULL(&hostPerRemoteDatacenter);
        ZVAL_NULL(&useRemoteDatacenterForLocalConsistencies);
    }

    if (self->local_rack != nullptr)
    {
        ZVAL_STR_COPY(&localRack, self->local_rack);
    }
    else
    {
        ZVAL_NULL(&localRack);
    }

    if (self->application_name != nullptr)
    {
        ZVAL_STR_COPY(&applicationName, self->application_name);
    }
    else
    {
        ZVAL_NULL(&applicationName);
    }

    if (self->application_version != nullptr)
    {
        ZVAL_STR_COPY(&applicationVersion, self->application_version);
    }
    else
    {
        ZVAL_NULL(&applicationVersion);
    }

    ZVAL_STRING(&reconnectPolicy,
                self->reconnect_policy == RECONNECT_POLICY_EXPONENTIAL ? "exponential" : "constant");
    /* Milliseconds internally, seconds here, matching reconnectInterval. */
    ZVAL_DOUBLE(&reconnectMaxInterval, (double)self->reconnect_max_interval / 1000);

    if (self->speculative_execution_delay > 0)
    {
        ZVAL_DOUBLE(&speculativeExecutionDelay, (double)self->speculative_execution_delay / 1000);
        ZVAL_LONG(&speculativeExecutionMax, self->speculative_execution_max);
    }
    else
    {
        ZVAL_NULL(&speculativeExecutionDelay);
        ZVAL_NULL(&speculativeExecutionMax);
    }

    ZVAL_BOOL(&useTokenAwareRouting, self->use_token_aware_routing);

    if (self->username)
    {
        ZVAL_STR_COPY(&username, self->username);
        /* The credential reaches var_dump/print_r/(array) and every framework
         * debug renderer from here, which would defeat the
         * #[\SensitiveParameter] on withCredentials(). Redact unless the
         * operator asked for the real value with cassandra.expose_credentials. */
        if (PHP_SCYLLADB_G(expose_credentials))
        {
            ZVAL_STR_COPY(&password, self->password);
        }
        else
        {
            ZVAL_STRINGL(&password, "***", 3);
        }
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
        ZVAL_DOUBLE(&tcpKeepalive, (double)self->tcp_keepalive_delay);
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
    (void)zend_hash_str_update(props, ZEND_STRL("localRack"), &localRack);
    (void)zend_hash_str_update(props, ZEND_STRL("applicationName"), &applicationName);
    (void)zend_hash_str_update(props, ZEND_STRL("applicationVersion"), &applicationVersion);
    (void)zend_hash_str_update(props, ZEND_STRL("reconnectPolicy"), &reconnectPolicy);
    (void)zend_hash_str_update(props, ZEND_STRL("reconnectMaxInterval"), &reconnectMaxInterval);
    (void)zend_hash_str_update(props, ZEND_STRL("speculativeExecutionDelay"), &speculativeExecutionDelay);
    (void)zend_hash_str_update(props, ZEND_STRL("speculativeExecutionMax"), &speculativeExecutionMax);
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

    if (self->local_rack != nullptr)
    {
        zend_string_release(self->local_rack);
        self->local_rack = nullptr;
    }

    if (self->application_name != nullptr)
    {
        zend_string_release(self->application_name);
        self->application_name = nullptr;
    }

    if (self->application_version != nullptr)
    {
        zend_string_release(self->application_version);
        self->application_version = nullptr;
    }

    if (self->local_address != nullptr)
    {
        zend_string_release(self->local_address);
        self->local_address = nullptr;
    }

    if (self->execution_profiles != nullptr)
    {
        zend_hash_destroy(self->execution_profiles);
        FREE_HASHTABLE(self->execution_profiles);
        self->execution_profiles = nullptr;
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

    const char *contact_points = PHP_SCYLLADB_G(contact_points);
    if (contact_points == nullptr || *contact_points == '\0')
    {
        contact_points = PHP_SCYLLADB_DEFAULT_CONTACT_POINTS;
    }

    /* Empty means "not set": the INI default is "" for each of these. */
#define PHP_SCYLLADB_SEED_STR(field, ini)                                          \
    do {                                                                           \
        const char *v = PHP_SCYLLADB_G(ini);                                       \
        self->field = (v != nullptr && *v != '\0') ? zend_string_init(v, strlen(v), 0) : nullptr; \
    } while (0)

    PHP_SCYLLADB_SEED_STR(local_dc, local_dc);
    PHP_SCYLLADB_SEED_STR(local_rack, local_rack);
    PHP_SCYLLADB_SEED_STR(application_name, application_name);
    PHP_SCYLLADB_SEED_STR(application_version, application_version);
    PHP_SCYLLADB_SEED_STR(local_address, local_address);
#undef PHP_SCYLLADB_SEED_STR

    self->connection_idle_timeout = (uint32_t)PHP_SCYLLADB_G(connection_idle_timeout);
    self->max_schema_wait_time = (uint32_t)PHP_SCYLLADB_G(max_schema_wait_time);
    self->resolve_timeout = (uint32_t)PHP_SCYLLADB_G(resolve_timeout);
    self->monitor_reporting_interval = (uint32_t)PHP_SCYLLADB_G(monitor_reporting_interval);
    self->queue_size_io = (uint32_t)PHP_SCYLLADB_G(queue_size_io);
    self->tracing_consistency = (uint32_t)PHP_SCYLLADB_G(tracing_consistency);
    self->tracing_max_wait_time = (uint32_t)PHP_SCYLLADB_G(tracing_max_wait_time);
    self->tracing_retry_wait_time = (uint32_t)PHP_SCYLLADB_G(tracing_retry_wait_time);
    self->prepare_on_all_hosts = (cass_bool_t)PHP_SCYLLADB_G(prepare_on_all_hosts);
    self->prepare_on_up_or_add_host = (cass_bool_t)PHP_SCYLLADB_G(prepare_on_up_or_add_host);
    self->shuffle_replicas = (cass_bool_t)PHP_SCYLLADB_G(shuffle_replicas);
    self->no_compact = (cass_bool_t)PHP_SCYLLADB_G(no_compact);
    self->beta_protocol = (cass_bool_t)PHP_SCYLLADB_G(beta_protocol);

    /* A local rack is what selects ScyllaDB rack-aware routing. */
    self->load_balancing_policy =
        self->local_rack != nullptr ? LOAD_BALANCING_RACK_AWARE : LOAD_BALANCING_DEFAULT;

    self->reconnect_policy =
        (PHP_SCYLLADB_G(reconnect_policy) != nullptr &&
         strcasecmp(PHP_SCYLLADB_G(reconnect_policy), "exponential") == 0)
            ? RECONNECT_POLICY_EXPONENTIAL
            : RECONNECT_POLICY_CONSTANT;

    self->reconnect_max_interval = (uint32_t)PHP_SCYLLADB_G(reconnect_max_interval);
    self->speculative_execution_delay = (uint32_t)PHP_SCYLLADB_G(speculative_execution_delay);
    self->speculative_execution_max = (int32_t)PHP_SCYLLADB_G(speculative_execution_max);
    self->coalesce_delay = (int32_t)PHP_SCYLLADB_G(coalesce_delay);
    self->new_request_ratio = (int32_t)PHP_SCYLLADB_G(new_request_ratio);

    self->contact_points = zend_string_init(contact_points, strlen(contact_points), 0);
    self->port = (uint16_t)PHP_SCYLLADB_G(port);
    self->used_hosts_per_remote_dc = 0;
    self->allow_remote_dcs_for_local_cl = cass_false;
    self->use_token_aware_routing = (cass_bool_t)PHP_SCYLLADB_G(token_aware_routing);
    self->username = nullptr;
    self->password = nullptr;
    self->connect_timeout = (uint32_t)PHP_SCYLLADB_G(connect_timeout);
    self->request_timeout = (uint32_t)PHP_SCYLLADB_G(request_timeout);
    self->default_consistency = (uint32_t)PHP_SCYLLADB_G(default_consistency);
    self->default_page_size = (uint32_t)PHP_SCYLLADB_G(default_page_size);
    /* cassandra.allow_persistent is a floor, not a default: withPersistentSessions(true)
     * must not be able to re-enable caching that the operator turned off. */
    self->persist = (cass_bool_t)PHP_SCYLLADB_G(allow_persistent);
    self->protocol_version = (uint32_t)PHP_SCYLLADB_G(protocol_version);
    self->io_threads = (uint32_t)PHP_SCYLLADB_G(io_threads);
    self->core_connections_per_host = (uint32_t)PHP_SCYLLADB_G(core_connections_per_host);
    self->max_connections_per_host = (uint32_t)PHP_SCYLLADB_G(max_connections_per_host);
    self->reconnect_interval = (uint32_t)PHP_SCYLLADB_G(reconnect_interval);
    self->enable_latency_aware_routing = (cass_bool_t)PHP_SCYLLADB_G(latency_aware_routing);
    self->enable_tcp_nodelay = (cass_bool_t)PHP_SCYLLADB_G(tcp_nodelay);
    self->enable_tcp_keepalive = PHP_SCYLLADB_G(tcp_keepalive_delay) > 0 ? cass_true : cass_false;
    self->tcp_keepalive_delay = (uint32_t)PHP_SCYLLADB_G(tcp_keepalive_delay);
    self->enable_schema = (cass_bool_t)PHP_SCYLLADB_G(schema_metadata);
    self->blacklist_hosts = nullptr;
    self->whitelist_hosts = nullptr;
    self->blacklist_dcs = nullptr;
    self->whitelist_dcs = nullptr;
    self->enable_hostname_resolution = (cass_bool_t)PHP_SCYLLADB_G(hostname_resolution);
    self->enable_randomized_contact_points = (cass_bool_t)PHP_SCYLLADB_G(randomized_contact_points);
    self->connection_heartbeat_interval = (uint32_t)PHP_SCYLLADB_G(connection_heartbeat_interval);
    self->timestamp_gen = nullptr;
    self->retry_policy = nullptr;
    self->ssl_options = nullptr;
    self->execution_profiles = nullptr;

    ZVAL_UNDEF(&self->default_timeout);

    return &self->zendObject;
}
