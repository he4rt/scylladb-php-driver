use phper::ini::Policy;
use phper::modules::Module;
use phper::php_get_module;



#[php_get_module]
pub fn get_module() -> Module {
    let mut module = Module::new(
        env!("CARGO_CRATE_NAME"),
        env!("CARGO_PKG_VERSION"),
        env!("CARGO_PKG_AUTHORS"),
    );

    // TODO: Create and issue on `phper` to support OnModify callback for INI
    module.add_ini("scylladb.log_level", "error".to_string(), Policy::All);
    module.add_ini("scylladb.log", "scylladb.log".to_string(), Policy::All);


    module.on_module_init(|| {});
    module.on_module_shutdown(|| {});
    module.on_request_init(|| {});
    module.on_request_shutdown(|| {});


    module
}
