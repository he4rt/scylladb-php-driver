use std::env::VarError;

fn find_php_config(cwd: impl AsRef<str>) -> String {
    match std::env::var("PHP_CONFIG") {
        Ok(value) => value,
        Err(VarError::NotPresent) => {
            let php_version = std::env::var("PHP_VERISON");

            if let Ok(php_version) = php_version {
                format!("{}/php/{php_version}/bin/php-config", cwd.as_ref())
            } else {
                format!("{}/php/bin/php-config", cwd.as_ref())
            }
        }
        Err(err) => panic!("Failed to get PHP_CONFIG Environment variable: {:?}", err),
    }
}

fn main() {
    let cwd = std::env::current_dir()
        .unwrap()
        .to_string_lossy()
        .to_string();

    let php_config = find_php_config(cwd);
    println!("cargo:rustc-env=PHP_CONFIG={php_config}");
    phper_build::register_configures();

    #[cfg(target_os = "macos")]
    {
        println!("cargo:rustc-link-arg=-undefined");
        println!("cargo:rustc-link-arg=dynamic_lookup");
    }
}
