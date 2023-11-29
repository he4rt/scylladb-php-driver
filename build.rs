use std::env::VarError;
use std::path::{PathBuf, Path};
use std::str::FromStr;

const PHP_VERSIONS: &[&str] = &["8.3", "8.2", "8.1"];
const PHP_RELEASES: &[&str] = &["release", "debug"];

fn check_for_releases(path: impl AsRef<Path>) -> Vec<String> {
    let mut sorted_versions = Vec::new();

    for version in PHP_VERSIONS {
        for release in PHP_RELEASES {
            let mut p = PathBuf::from(path.as_ref());
            p.push(format!("{version}-{release}-nts"));

            if p.exists() {
                let string = p.as_path().to_string_lossy().to_string();
                sorted_versions.push(string);
            }
        }
    }

    sorted_versions
}

fn find_php_config(cwd: impl AsRef<str>) -> String {
    match std::env::var("PHP_CONFIG") {
        Ok(value) => {
            println!("cargo:rerun-if-env-changed=PHP_CONFIG");
            value
        },
        Err(VarError::NotPresent) => {
            let php_version = std::env::var("PHP_VERISON");

            if let Ok(php_version) = php_version {
                format!("{}/php/{php_version}/bin/php-config", cwd.as_ref())
            } else {
                let path = PathBuf::from_str(cwd.as_ref()).unwrap();
                let p = path.as_os_str().to_string_lossy().to_string();
                let sorterd_versions = check_for_releases(p);

                println!("{:?}", sorterd_versions);

                if sorterd_versions.len() > 0 {
                    sorterd_versions[0].clone()
                } else {
                    format!("{}/php/bin/php-config", cwd.as_ref())
                }
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
