use std::future::Future;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;

use parking_lot::{Condvar, Mutex};
use tokio::runtime::{Handle as TokioHandle, Runtime as TokioRuntime};
use tokio::task::AbortHandle;

#[derive(Clone)]
pub struct Handle(TokioHandle);

pub struct PhpFuture<T>(Arc<(Mutex<Option<T>>, Condvar, AtomicBool)>);

impl<T> super::PhpFuture<T> for PhpFuture<T> {
    fn ready(&self) -> bool {
        let (_, _, ready) = &*self.0;

        ready.load(Ordering::Relaxed)
    }

    unsafe fn value(self) -> T {
        let (val, _, _) = Arc::into_inner(self.0).unwrap();

        val.into_inner().unwrap()
    }

    fn wait(self) -> T {
        {
            let (guard, condvar, _) = &*self.0;
            condvar.wait_while(&mut guard.lock(), |opt| opt.is_some());
        }

        unsafe { self.value() }
    }
}

pub struct Abort(AbortHandle);

impl super::Abortable for Abort {
    fn abort(&self) {
        self.0.abort();
    }
}

impl super::Handle for Handle {
    type Output<T> = PhpFuture<T>;
    type Abortable = Abort;
    fn spawn<F>(&self, fut: F) -> Self::Output<F::Output>
    where
        F: Future + Send + 'static,
        <F as std::future::Future>::Output: Send + 'static,
    {
        let future = PhpFuture(Arc::new((
            parking_lot::Mutex::new(None),
            parking_lot::Condvar::new(),
            AtomicBool::new(false),
        )));

        let l1 = Arc::clone(&future.0);
        self.0.spawn(async move {
            let val = fut.await;

            let (lock, condvar, ready) = &*l1;
            let mut v = lock.lock();
            *v = Some(val);
            condvar.notify_one();
            ready.store(true, Ordering::Relaxed);
        });

        future
    }

    fn spawn_with_callback<F, Fn>(&self, fut: F, cb: Fn) -> Self::Abortable
    where
        F: Future + Send + 'static,
        <F as std::future::Future>::Output: Send + 'static,
        Fn: FnOnce(F::Output) + Send + 'static,
    {
        let handle = self.0.spawn(async move {
            let val = fut.await;

            cb(val);
        });

        Abort(handle.abort_handle())
    }
}

pub struct Runtime {
    handle: Handle,
    thread_handle: std::thread::JoinHandle<()>,
    sender: tokio::sync::oneshot::Sender<()>,
}

impl Runtime {
    pub fn new() -> std::io::Result<Self> {
        let runtime = TokioRuntime::new()?;
        let handle = Handle(runtime.handle().clone());

        let (sender, recv) = tokio::sync::oneshot::channel::<()>();

        let thread_handle = std::thread::spawn(move || {
            runtime.block_on(async {
                let _ = recv.await;
            });
        });

        Ok(Runtime {
            handle,
            sender,
            thread_handle,
        })
    }
}

impl super::Runtime for Runtime {
    type T = Handle;

    fn stop(self) -> Result<(), Box<dyn std::error::Error>> {
        // Safety: Send -> Never fails -> Safe to unwrap
        self.sender.send(()).unwrap();

        if self.thread_handle.join().is_ok() {
            Ok(())
        } else {
            Err("failed to join thread handle".into())
        }
    }

    fn handle(&self) -> Self::T {
        self.handle.clone()
    }
}
