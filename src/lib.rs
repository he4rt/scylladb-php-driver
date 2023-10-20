use std::cell::Cell;
use std::rc::Rc;
use std::sync::Arc;
use tokio::runtime::Runtime;

use phper::ini::Policy;
use phper::modules::Module;
use phper::php_get_module;
use tokio::sync::oneshot;

use crate::cluster::make_cluster_builder_class;

mod cluster;

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

    let rt = Arc::new(Runtime::new().unwrap());
    let rt1 = Arc::clone(&rt);

    let handle = Rc::new(Cell::new(None));
    let handle1 = Rc::clone(&handle);

    let (sx, rx) = oneshot::channel::<()>();

    module.on_module_init(move || {
        handle.set(Some(std::thread::spawn(move || {
            rt.block_on(async {
                let _ = rx.await;
            });
        })));
    });

    module.on_module_shutdown(move || {
        drop(sx);

        match handle1.take() {
            Some(handle) => handle.join().unwrap(),
            None => {}
        };
    });

    module.add_class(make_cluster_builder_class(rt1.handle().clone()));

    module
}
