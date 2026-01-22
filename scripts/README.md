# Scripts Directory

This directory contains helper scripts for building, testing, and managing the ScyllaDB PHP Driver project.

## Testing & CI Scripts

### `local-test.sh` - Local Test Runner (NEW)
**Purpose**: Run tests locally that mirror the GitHub Actions CI workflow.

**Usage**:
```bash
# Test everything (ZendCPP + Extension)
./scripts/local-test.sh all

# Test only ZendCPP
./scripts/local-test.sh zendcpp

# Test only extension
./scripts/local-test.sh extension

# Test with specific PHP version and driver
./scripts/local-test.sh extension 8.4 cassandra
./scripts/local-test.sh all 8.3 scylladb
```

**Features**:
- Builds and caches dependencies automatically
- Supports both ScyllaDB and Cassandra drivers
- Fast iteration during development
- Mimics CI environment

---

### `clean.sh` - Build Artifact Cleaner (NEW)
**Purpose**: Clean build artifacts and caches.

**Usage**:
```bash
# Clean everything
./scripts/clean.sh all

# Clean only extension build
./scripts/clean.sh extension

# Clean only ZendCPP build
./scripts/clean.sh zendcpp

# Clean dependencies (interactive)
./scripts/clean.sh deps

# Clean CMake artifacts
./scripts/clean.sh cmake

# Clean Composer dependencies
./scripts/clean.sh composer
```

---

## Legacy Build Scripts

### `build.sh` - Extension Builder
**Purpose**: Build the PHP extension using autoconf/configure.

**Usage**:
```bash
./scripts/build.sh [driver] [version] [php-version] [php-dir]

# Examples
./scripts/build.sh scylladb
./scripts/build.sh cassandra master 8.4
```

**Note**: This script is still used by `local-test.sh` for building the extension.

---

### `compile-php.sh` - PHP Compiler
**Purpose**: Compile PHP from source with specific options.

**Usage**:
```bash
./scripts/compile-php.sh -v VERSION -o OUTPUT_DIR -s SANITIZERS -d DEBUG -z THREAD_MODEL -a
```

**Note**: This script was primarily used for Docker builds. The new CI uses pre-built PHP from `shivammathur/setup-php`.

---

### `compile-libuv.sh` - libuv Compiler
**Purpose**: Compile libuv from source.

**Usage**:
```bash
./scripts/compile-libuv.sh
```

**Note**: The new CI builds libuv directly in the workflow. This script can still be used for local builds.

---

### `compile-cpp-driver.sh` - C++ Driver Compiler
**Purpose**: Compile ScyllaDB or Cassandra C++ driver from source.

**Usage**:
```bash
./scripts/compile-cpp-driver.sh [scylladb|cassandra]
```

**Note**: The new CI builds drivers directly in the workflow. This script can still be used for local builds.

---

## Docker Scripts

### `run-docker-tests.sh` - Docker Test Runner
**Purpose**: Run tests inside Docker container.

**Note**: This script is deprecated in favor of the new CI workflow. It's kept for backwards compatibility and local development if needed.

---

### `run-scylladb.sh` - ScyllaDB Launcher
**Purpose**: Start ScyllaDB container for local testing.

**Usage**:
```bash
./scripts/run-scylladb.sh
```

---

### `run-scylladb-ssl.sh` - ScyllaDB SSL Launcher
**Purpose**: Start ScyllaDB container with SSL enabled.

**Usage**:
```bash
./scripts/run-scylladb-ssl.sh
```

---

### `run-tests.sh` - Legacy Test Runner
**Purpose**: Run tests in the old Docker-based workflow.

**Note**: Deprecated in favor of `local-test.sh` for local testing and GitHub Actions CI for automated testing.

---

## Setup Scripts

### `setup` - Project Setup (if exists)
**Purpose**: Initial project setup and configuration.

---

## Quick Reference

### Development Workflow

1. **First time setup**:
   ```bash
   ./scripts/local-test.sh all
   ```

2. **Working on ZendCPP**:
   ```bash
   # Make changes to ZendCPP code
   ./scripts/local-test.sh zendcpp
   ```

3. **Working on extension**:
   ```bash
   # Make changes to extension code
   ./scripts/local-test.sh extension
   ```

4. **Clean rebuild**:
   ```bash
   ./scripts/clean.sh extension
   ./scripts/local-test.sh extension
   ```

5. **Full clean and rebuild**:
   ```bash
   ./scripts/clean.sh all
   ./scripts/local-test.sh all
   ```

### CI Workflow

The GitHub Actions CI automatically:
- Detects what changed (ZendCPP, extension, dependencies)
- Builds and caches dependencies
- Runs appropriate tests in parallel
- Tests multiple PHP versions (8.1, 8.2, 8.3, 8.4)
- Tests both NTS and ZTS builds
- Tests both ScyllaDB and Cassandra drivers

See [../.github/workflows/CI_TESTING.md](../.github/workflows/CI_TESTING.md) for details.

---

## Migration from Docker to Native CI

### What Changed

**Removed from CI**:
- ❌ Docker image building in CI
- ❌ Custom PHP compilation in CI
- ❌ Slow iterative testing

**Added**:
- ✅ Native GitHub Actions workflow
- ✅ Fast PHP setup with `shivammathur/setup-php`
- ✅ Intelligent change detection
- ✅ Dependency caching
- ✅ Parallel test execution

**Still Available for Local Development**:
- ✅ Docker images (for consistent environments)
- ✅ All legacy scripts (for specific use cases)
- ✅ Manual builds

---

## Tips

1. **Use `local-test.sh` for development** - It's faster and mimics CI
2. **Use `clean.sh` when switching branches** - Avoids stale build artifacts
3. **Keep dependencies cached** - Only clean deps when necessary
4. **Run tests before pushing** - Catch issues early
5. **Check CI logs** - If CI fails, check which matrix job failed and reproduce locally

---

## Getting Help

- **CI Documentation**: See [../.github/workflows/CI_TESTING.md](../.github/workflows/CI_TESTING.md)
- **Quick Reference**: See [../.github/workflows/QUICK_REFERENCE.md](../.github/workflows/QUICK_REFERENCE.md)
- **Issues**: Open a GitHub issue with the `ci` or `build` label

---

**Last updated**: January 2026
