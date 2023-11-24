pub mod runtimes;

pub trait Driver<T: runtimes::Handle> {
    fn register(&mut self, handle: T);
}
