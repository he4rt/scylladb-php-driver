use std::future::Future;

pub mod tokio;

pub trait Runtime {
    type T: Handle;

    fn handle(&self) -> Self::T;
    fn stop(self) -> Result<(), Box<dyn std::error::Error>>;
}

pub trait PhpFuture<T> {
    fn ready(&self) -> bool;
    fn wait(self) -> T;

    /// # Safety
    /// This function is safe only if ready() retuns true
    /// since it consumes the Future itself, it will drop Mutex and Condvar
    unsafe fn value(self) -> T;
}

pub trait Abortable {
    fn abort(&self);
}

pub trait Handle: Clone {
    type Output<T>: PhpFuture<T>;
    type Abortable: Abortable;

    fn spawn<F>(&self, fut: F) -> Self::Output<F::Output>
    where
        F: Future + Send + 'static,
        <F as std::future::Future>::Output: Send + 'static;

    fn spawn_with_callback<F, Fn>(&self, fut: F, cb: Fn) -> Self::Abortable
    where
        F: Future + Send + 'static,
        <F as std::future::Future>::Output: Send + 'static,
        Fn: FnOnce(F::Output) + Send + 'static;
}

pub fn spawn() -> std::io::Result<tokio::Runtime> {
    tokio::Runtime::new()
}
