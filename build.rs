fn main() {
    let cwd = std::env::current_dir()
        .unwrap()
        .to_string_lossy()
        .to_owned()
        .to_string();

    println!("cargo:rustc-env=PHP_CONFIG={cwd}/php/bin/php-config");
    phper_build::register_configures();

    #[cfg(target_os = "macos")]
    {
        println!("cargo:rustc-link-arg=-undefined");
        println!("cargo:rustc-link-arg=dynamic_lookup");
    }
}
