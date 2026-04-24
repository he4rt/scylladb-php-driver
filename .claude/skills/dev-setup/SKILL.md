---
name: dev-setup
description: Set up the ScyllaDB PHP driver for local development. Walks through installing all native dependencies, building PHP, compiling the extension, and verifying everything works. Use when onboarding or setting up a new machine.
allowed-tools: Bash Read Glob
---

Set up the ScyllaDB PHP driver for local development on this machine.

Detect the OS and architecture first, then walk through every step. Do not skip steps — check what is already installed before running install commands.

## Step 0 — Detect environment

```bash
uname -s   # Darwin = macOS, Linux = Linux
uname -m   # arm64 = Apple Silicon, x86_64 = Intel/AMD
```

Report: OS, arch, whether Homebrew is installed (macOS), which PHP versions exist under `./php/`.

---

## Step 1 — System prerequisites

### macOS
```bash
# Install Homebrew if missing
/bin/bash -c "$(curl -fsSL https://brew.sh/install.sh)"

brew install cmake ninja pkg-config openssl zlib curl readline \
             libffi oniguruma libsodium gmp autoconf re2c bison \
             libssh2 ccache

# Add bison to PATH (macOS ships an old version)
echo 'export PATH="/opt/homebrew/opt/bison/bin:$PATH"' >> ~/.zshrc   # arm64
# or: echo 'export PATH="/usr/local/opt/bison/bin:$PATH"' >> ~/.zshrc  # intel
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential pkg-config cmake ninja-build ccache git curl \
    libssl-dev zlib1g-dev libcurl4-openssl-dev libreadline-dev \
    libffi-dev libonig-dev libsodium-dev libgmp-dev libssh2-1-dev \
    libxml2-dev libicu-dev libsqlite3-dev libzip-dev \
    bison re2c autoconf
```

### Linux (Fedora)
```bash
sudo dnf install -y \
    gcc gcc-c++ cmake ninja-build pkg-config ccache git curl \
    openssl-devel zlib-devel libcurl-devel readline-devel \
    libffi-devel oniguruma-devel libsodium-devel gmp-devel libssh2-devel \
    libxml2-devel libicu-devel sqlite-devel libzip-devel \
    bison re2c autoconf
```

Verify:
```bash
cmake --version      # need 3.30+
ninja --version
pkg-config --version
```

If cmake < 3.30, install from Kitware (Linux) or `brew install cmake` (macOS).

---

## Step 2 — libuv

```bash
./scripts/compile-libuv.sh --prefix ~/.local
```

macOS with Homebrew: skip this — `brew install libuv` is sufficient and automatically in pkg-config path.

Verify:
```bash
pkg-config --modversion libuv   # should print version
```

---

## Step 3 — ScyllaDB C++ driver

```bash
./scripts/compile-cpp-driver.sh --driver scylladb --prefix ~/.local
```

For DataStax/Cassandra driver (optional, only if you need to test against Cassandra):
```bash
./scripts/compile-cpp-driver.sh --driver cassandra --prefix ~/.local
```

Verify:
```bash
pkg-config --modversion scylla-cpp-driver
```

**macOS note**: `CASS_USE_TIMERFD` is automatically set to OFF by the script.

---

## Step 4 — PHP development builds

Build debug + NTS PHP for each version you want to develop against.
The builds land in `./php/<version>-<debug|release>-<nts|zts>/`.

```bash
# Debug build (recommended for development — slower but better error messages)
./scripts/compile-php.sh -v 8.4 -d -o ./php

# Release build (for performance testing)
./scripts/compile-php.sh -v 8.4 -o ./php

# ZTS (thread-safe) if needed
./scripts/compile-php.sh -v 8.4 -d -z -o ./php
```

Verify:
```bash
./php/8.4-debug-nts/bin/php --version
./php/8.4-debug-nts/bin/php-config --extension-dir
```

---

## Step 5 — Generate CMake presets

```bash
php generate-presets.php
```

This writes `CMakePresets.json` covering all PHP version × build type × ZTS/NTS combinations.

---

## Step 6 — Build the extension

List available presets:
```bash
cmake --list-presets
```

Build for your target (example: debug PHP 8.4 NTS):
```bash
cmake --preset DebugPHP8.4NTS
cmake --build out/DebugPHP8.4NTS --parallel
```

Set `PKG_CONFIG_PATH` if your dependencies are in `~/.local`:
```bash
export PKG_CONFIG_PATH="$HOME/.local/lib/pkgconfig:$PKG_CONFIG_PATH"
cmake --preset DebugPHP8.4NTS
cmake --build out/DebugPHP8.4NTS --parallel
```

On macOS, Homebrew prefix is auto-detected by CMakeLists.txt — no manual PKG_CONFIG_PATH needed.

Verify the extension was built:
```bash
ls out/DebugPHP8.4NTS/cassandra.so
```

---

## Step 7 — Install the extension into PHP

```bash
cmake --install out/DebugPHP8.4NTS --component extension
```

This installs `cassandra.so` to `$(php-config --extension-dir)`.

Verify it loads:
```bash
./php/8.4-debug-nts/bin/php -d extension=cassandra -r "echo Cassandra::VERSION;"
```

---

## Step 8 — Start ScyllaDB locally

```bash
./scripts/run-scylladb.sh        # starts ScyllaDB via docker compose
# or with SSL:
docker compose -f ./docker/docker-compose.ssl.yml up -d
```

Wait ~30s for ScyllaDB to be ready:
```bash
docker exec scylladb nodetool status
# Look for "UN" (Up/Normal) in the output
```

---

## Step 9 — Run the tests

```bash
cd tests
composer install
php -d extension=cassandra ./vendor/bin/pest --colors=always
```

With explicit ScyllaDB host:
```bash
SCYLLADB_HOSTS=127.0.0.1 php -d extension=cassandra ./vendor/bin/pest
```

---

## Step 10 — IDE setup

**VS Code / Cursor**: Install the clangd extension. The build generates `compile_commands.json`:
```bash
cmake --preset DebugPHP8.4NTS -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ln -sf out/DebugPHP8.4NTS/compile_commands.json compile_commands.json
```

**CLion**: Open the project — CMake presets are auto-detected. Select the desired preset in the CMake profile.

**JetBrains (PhpStorm)**: The extension stubs in `src/**/*.stub.php` provide IDE completion for the PHP API.

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| `pkg-config: command not found` | `brew install pkg-config` / `apt install pkg-config` |
| `Could NOT find Libuv` | Set `PKG_CONFIG_PATH` to include libuv pkgconfig dir |
| `Cannot find -l:libuv.a` | Rebuild libuv with `./scripts/compile-libuv.sh --prefix ~/.local` |
| `Could NOT find LibGMP` | `brew install gmp` / `apt install libgmp-dev` |
| `php-config not found` | Run Step 4 first, or set `CUSTOM_PHP_CONFIG` cmake variable |
| `cmake: version 3.X < 3.30` | `brew upgrade cmake` or install from cmake.org |
| ARM Mac: `-march=x86-64-v2` error | Should not happen — CMakeLists.txt auto-detects ARM and uses `native` |
| macOS: `-ggdb` compile error | Should not happen — compiler flags are platform-guarded |
| ScyllaDB not starting | Check Docker is running: `docker ps`. On Apple Silicon use Docker Desktop ≥ 4.x |

---

## Quick reference — full setup from scratch

```bash
# macOS (Apple Silicon / Intel)
brew install cmake ninja pkg-config openssl zlib curl readline \
             libffi oniguruma libsodium gmp autoconf re2c bison \
             libssh2 ccache libuv

./scripts/compile-cpp-driver.sh --driver scylladb --prefix /opt/homebrew
./scripts/compile-php.sh -v 8.4 -d -o ./php
php generate-presets.php
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"
cmake --preset DebugPHP8.4NTS && cmake --build out/DebugPHP8.4NTS --parallel
cmake --install out/DebugPHP8.4NTS --component extension
./scripts/run-scylladb.sh
cd tests && composer install && php -d extension=cassandra ./vendor/bin/pest
```

```bash
# Linux (Ubuntu)
sudo apt-get install -y build-essential pkg-config cmake ninja-build ccache \
    libssl-dev zlib1g-dev libcurl4-openssl-dev libreadline-dev libffi-dev \
    libonig-dev libsodium-dev libgmp-dev libssh2-1-dev libxml2-dev \
    libicu-dev libsqlite3-dev libzip-dev bison re2c autoconf

./scripts/setup   # installs libuv + cpp-driver + PHP builds

php generate-presets.php
cmake --preset DebugPHP8.4NTS && cmake --build out/DebugPHP8.4NTS --parallel
cmake --install out/DebugPHP8.4NTS --component extension
./scripts/run-scylladb.sh
cd tests && composer install && php -d extension=cassandra ./vendor/bin/pest
```
