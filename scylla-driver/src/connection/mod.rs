// use std::net::{IpAddr, Ipv4Addr, SocketAddr};
// use std::time::Duration;

// use phper::classes::ClassEntity;
// use phper::objects::StateObj;
// use phper::values::ZVal;

// use driver_common::runtimes::Handle;

// pub struct ClusterBuilder<T> {
//     pub(self) handle: T,
// }

// impl<T: Handle> ClusterBuilder<T> {
//     fn new(handle: T) -> Self {
//         Self { handle }
//     }
// }

// pub const CONNECTION_BUILDER_CLASS_NAME: &str = "ScyllaDB\\Connection\\Builder";
// pub const CONNECTION_CLASS_NAME: &str = "ScyllaDB\\Connection\\Connection";

//  fn build<T: Handle>(
//     this: &mut StateObj<ClusterBuilder<T>>,
//     _: &mut [ZVal],
// ) -> Result<(), phper::Error> {
//     // let state = this.as_state();

//     // let builder = SessionBuilder::new()
//     //     .known_nodes(&state.hosts)
//     //     .connection_timeout(Duration::from_secs(3))
//     //     .cluster_metadata_refresh_interval(Duration::from_secs(10));

//     // let result = state.handle.block_on(async move {
//     //     let session = builder.build().await.unwrap();

//     //     session.query("SELECT * FROM system.clients", &[]).await
//     // });

//     // echo!("{:?}", result);

//     Ok(())
// }

// pub(crate) fn make_cluster_builder_class<T: Handle>(handle: T) -> ClassEntity<ClusterBuilder<T>> {
//     let mut class: ClassEntity<ClusterBuilder<T>> =
//         ClassEntity::new_with_state_constructor(CONNECTION_BUILDER_CLASS_NAME, move || {
//             ClusterBuilder::new(handle.clone())
//         });

//     class.add_method("build", phper::classes::Visibility::Public, build::<T>);

//     class
// }
