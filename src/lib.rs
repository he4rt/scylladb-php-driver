use phper::ini::Policy;
use phper::modules::Module;
use phper::php_get_module;

use driver_common::runtimes::Runtime;
use driver_common::Driver;

#[cfg(all(feature = "scylla", feature = "cassandra"))]
compile_error!("feature \"scylla\" and feature \"cassandra\" cannot be enabled at the same time");

#[cfg(all(not(feature = "scylla"), not(feature = "cassandra")))]
compile_error!("feature \"scylla\" or feature \"cassandra\" must be enabled for compilation");

#[php_get_module]
pub fn get_module() -> Module {
    let mut module = Module::new(
        env!("CARGO_CRATE_NAME"),
        env!("CARGO_PKG_VERSION"),
        env!("CARGO_PKG_AUTHORS"),
    );

    #[cfg(feature = "cassandra")]
    let _d = {
        module.add_ini("cassandra.log_level", "error".to_string(), Policy::All);
        module.add_ini(
            "cassandra.log",
            "/var/log/php-cassandra/cassandra.log".to_string(),
            Policy::All,
        );
    };

    #[cfg(feature = "scylla")]
    let mut d = {
        module.add_ini("scylladb.log_level", "error".to_string(), Policy::All);
        module.add_ini(
            "scylladb.log",
            "/var/log/php-scylladb/scylladb.log".to_string(),
            Policy::All,
        );

        scylla_driver::ScyllaDBDriver
    };

    let runtime = driver_common::runtimes::spawn().unwrap();

    d.register(runtime.handle());

    module.on_module_shutdown(move || {
        if let Err(inner) = runtime.stop() {
            phper::error!("Failed to stop Runtime: {:?}", inner);
        }
    });

    module
}
