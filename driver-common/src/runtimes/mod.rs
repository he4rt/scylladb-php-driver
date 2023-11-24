pub mod tokio;

pub trait Runtime {
    type T: Handle;

    fn handle(&self) -> Self::T;
    fn stop(self) -> Result<(), Box<dyn std::error::Error>> ;
}

pub trait Handle: Clone {}

pub fn spawn() -> std::io::Result<tokio::Runtime> {
    tokio::Runtime::new()
}
