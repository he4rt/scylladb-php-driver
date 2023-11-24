use driver_common::Driver;

mod cluster_builder;

#[derive(Default)]
pub struct ScyllaDBDriver;

impl<T> Driver<T> for ScyllaDBDriver
where
    T: driver_common::runtimes::Handle + 'static,
{
    fn register(&mut self, handle: T) {
        cluster_builder::make_cluster_builder_class::<T>(handle.clone());
    }
}
