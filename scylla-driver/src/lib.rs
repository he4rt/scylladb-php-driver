use driver_common::Driver;

mod connection;

#[derive(Default)]
pub struct ScyllaDBDriver;

impl<T> Driver<T> for ScyllaDBDriver
where
    T: driver_common::runtimes::Handle + 'static,
{
    fn register(&mut self, _handle: T) {
        // connection::make_cluster_builder_class::<T>(handle.clone());
    }
}
