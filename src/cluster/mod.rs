use std::net::{IpAddr, Ipv4Addr, SocketAddr};
use std::time::Duration;

use phper::classes::ClassEntity;
use phper::echo;
use phper::objects::StateObj;
use phper::values::ZVal;
use scylla::statement::Consistency;

//     zend_string *blacklist_hosts;
//     zend_string *whitelist_hosts;
//     zend_string *blacklist_dcs;
//     zend_string *whitelist_dcs;

//     uint32_t used_hosts_per_remote_dc;
//     uint32_t connect_timeout;
//     uint32_t default_consistency;
//     uint32_t protocol_version;
//     uint32_t io_threads;
//     uint32_t core_connections_per_host;
//     uint32_t max_connections_per_host;
//     uint32_t reconnect_interval;
//     uint32_t tcp_keepalive_delay;
//     cass_bool_t enable_latency_aware_routing;
//     cass_bool_t enable_tcp_nodelay;
//     cass_bool_t enable_tcp_keepalive;
//     cass_bool_t enable_schema;
//     cass_bool_t enable_hostname_resolution;
//     cass_bool_t enable_randomized_contact_points;
//     cass_bool_t allow_remote_dcs_for_local_cl;
//     cass_bool_t use_token_aware_routing;
//     uint32_t connection_heartbeat_interval;
//     uint32_t request_timeout;
//     uint16_t port;
//     php_driver_load_balancing load_balancing_policy;
//     cass_bool_t persist;
//     php_driver_retry_policy *retry_policy;
//     php_driver_timestamp_gen *timestamp_gen;
//     php_driver_ssl *ssl_options;
//     zval default_timeout;

pub struct ClusterBuilder {
    pub(self) handle: Handle,

    pub(self) persist: bool,
    pub(self) default_consistency: Consistency,
    pub(self) default_page_size: i32,
    pub(self) hosts: Vec<String>,
    pub(self) username: String,
    pub(self) password: String,
    pub(self) datacenter: String,
}

impl ClusterBuilder {
    fn new(handle: Handle) -> Self {
        Self {
            handle,
            hosts: vec!["127.0.0.1:9042".to_string()],
            persist: true,
            default_page_size: 4096,
            username: "".to_string(),
            password: "".to_string(),
            datacenter: "datacenter1".to_string(),
            default_consistency: Consistency::LocalOne,
        }
    }
}

pub const CLUSTER_BUILDER_CLASS_NAME: &'static str = "ScyllaDB\\Cluster\\Builder";

use scylla::SessionBuilder;
use tokio::runtime::Handle;

pub(self) fn build(
    this: &mut StateObj<ClusterBuilder>,
    _: &mut [ZVal],
) -> Result<(), phper::Error> {
    let state = this.as_state();

    let builder = SessionBuilder::new()
        .known_nodes(&state.hosts)
        .connection_timeout(Duration::from_secs(3))
        .cluster_metadata_refresh_interval(Duration::from_secs(10));

    let result = state.handle.block_on(async move {
        let session = builder.build().await.unwrap();

        session.query("SELECT * FROM system.clients", &[]).await
    });

    echo!("{:?}", result);

    Ok(())
}

pub(crate) fn make_cluster_builder_class(handle: Handle) -> ClassEntity<ClusterBuilder> {
    let mut class: ClassEntity<ClusterBuilder> =
        ClassEntity::new_with_state_constructor(CLUSTER_BUILDER_CLASS_NAME, move || {
            ClusterBuilder::new(handle.clone())
        });

    class.add_method("build", phper::classes::Visibility::Public, build);

    class
}
