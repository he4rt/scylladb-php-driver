# Building Cassandra Driver

In order to build PHP Cassandra driver from source, two things are neccessary.

1. LibUV
2. cpp-driver (Datastax C/C++ driver)

### 1. Building and Installing Latest LibUV

#### Required Build Tools

```bash
  sudo apt-get install build-essential cmake pkg-config ninja-build
```

#### Required Packages

```bash
    sudo apt-get install libssl-dev
```

```bash
    cd lib/libuv/
    mkdir -p build
    cmake .. -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DLIBUV_BUILD_TESTS=OFF \
      -DLIBUV_BUILD_BENCH=OFF \
      -DCMAKE_C_FLAGS="-fPIC"
    ninja
    sudo ninja install
```

### 2. Building and Installing Datastax C/C++ Driver

#### Required Build Tools

```bash
  sudo apt-get install build-essential cmake pkg-config ninja-build
```

#### Required Packages

```bash
    sudo apt-get install libssl-dev libgmp-dev  openssl zlib1g-dev libpcre3-dev
```

```bash
    cd lib/cpp-driver/
    mkdir -p build
    cmake .. -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS="-fPIC" \
        -DCMAKE_C_FLAGS="-fPIC" \
        -DCASS_BUILD_STATIC=ON \
        -DCASS_BUILD_SHARED=OFF \
        -DCASS_USE_TIMERFD=ON \
        -DCASS_USE_STATIC_LIBS=ON \
        -DCASS_USE_OPENSSL=ON
    ninja
    sudo ninja install
```

### 3. Building and Installing PHP Cassandra Driver

#### Required Build Tools

```bash
  sudo apt-get install build-essential cmake pkg-config ninja-build
```

#### Required Packages

```bash
    sudo apt-get install libssl-dev libgmp-dev
```

```bash
    cd ../../
    mkdir -p build
    cmake .. -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DLINK_STATICALLY=ON
    ninja
    sudo ninja install
```
