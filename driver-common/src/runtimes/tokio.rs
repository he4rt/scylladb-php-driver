use std::thread::JoinHandle;

use tokio::runtime::{Handle as TokioHandle, Runtime as TokioRuntime};
use tokio::sync::oneshot;

#[derive(Clone)]
pub struct Handle(TokioHandle);

impl super::Handle for Handle {}

pub struct Runtime {
    handle: Handle,
    thread_handle: JoinHandle<()>,
    sender: oneshot::Sender<()>,
}

impl Runtime {
    pub fn new() -> std::io::Result<Self> {
        let runtime = TokioRuntime::new()?;
        let handle = Handle(runtime.handle().clone());
        let (sender, recv) = oneshot::channel::<()>();

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
