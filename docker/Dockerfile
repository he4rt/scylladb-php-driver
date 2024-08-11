ARG UBUNTU_VERSION="24.04"

FROM ubuntu:$UBUNTU_VERSION AS base

ARG LIBUV_VERSION="v1.48.0"
ARG LIBUV_BUILD_TYPE="RelWithInfo"
ARG LIBUV_REPO="https://github.com/libuv/libuv.git"

ENV CC="gcc"
ENV CXX="g++"

ENV CFLAGS="-fPIC"
ENV CXXFLAGS="-fPIC"
ENV CMAKE_CXX_COMPILER_LAUNCHER="ccache"
ENV PATH="$PATH:/usr/local/bin"
ENV LD_LIBRARY_PATH="/lib/x86_64-linux-gnu:/usr/local/lib:/usr/lib:/lib:/lib64:/usr/local/lib/x86_64-linux-gnu"
ENV PKG_CONFIG_PATH="/usr/local/lib/pkgconfig"
ENV DEBIAN_FRONTEND="noninteractive"
ENV TZ="UTC"

RUN ln -snf "/usr/share/zoneinfo/$TZ" /etc/localtime \
    && echo "$TZ" > /etc/timezone \
    && apt-get update -y \
    && CODENAME=$(grep VERSION_CODENAME /etc/os-release | cut -d= -f2); [ "$CODENAME" = "focal" ] && apt-get install -y libasan6 \
    || apt-get install -y libasan8 \
    && apt-get install -y \
    pkg-config \
    ccache \
    wget \
    git \
    gcc \
    g++ \
    clang \
    build-essential \
    ninja-build \
    libubsan1 \
    libssl-dev \
    zlib1g-dev \
    libubsan1 \
    cmake

RUN git clone --depth 1 $LIBUV_REPO /libuv-src  \
    && cd libuv-src \
    && git fetch --tags \
    && git checkout -b $LIBUV_VERSION tags/$LIBUV_VERSION \
    && mkdir build \
    && cd build \
    && LDFLAGS="-flto -g" cmake -G Ninja \
    -DBUILD_TESTING=OFF \
    -DBUILD_BENCHMARKS=OFF \
    -DLIBUV_BUILD_SHARED=ON \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_BUILD_TYPE="$LIBUV_BUILD_TYPE" .. \
    && ninja install \
    && cd .. rm -rf build

FROM base AS php_build

ARG PHP_VERSION="8.3.8"
ARG PHP_ZTS="no"
ARG PHP_DEBUG="no"
ARG PHP_MEM_SANITIZERS="no"

COPY ./scripts /tmp/scripts

RUN apt-get -y install \
    bison \
    autoconf \
    re2c \
    libxml2-dev \
    libicu-dev \
    libsqlite3-dev \
    zlib1g-dev \
    libcurl4-openssl-dev \
    libffi-dev \
    libonig-dev \
    libzip-dev

ENV PATH="$PATH:/php/bin"

RUN /tmp/scripts/compile-php.sh -v $PHP_VERSION -o /php -s $PHP_MEM_SANITIZERS -d $PHP_DEBUG -z $PHP_ZTS \
    && php -r "copy('https://getcomposer.org/installer', 'composer-setup.php');" \
    && php composer-setup.php --install-dir=/php/bin --filename=composer \
    && chmod +x /php/bin/composer \
    && php -r "unlink('composer-setup.php');" \
    && rm -rf /tmp/scripts

COPY ./docker/php.ini /php/lib/php.ini

FROM base AS scylladb_cpp_driver_build

ARG SCYLLADB_DRIVER_BUILD_TYPE="RelWithInfo"
ARG SCYLLADB_DRIVER_REPO="https://github.com/scylladb/cpp-driver.git"

ENV CXXFLAGS="-fPIC -Wno-error=redundant-move"
ENV LDFLAGS="-flto -g"

RUN git clone --depth 1 $SCYLLADB_DRIVER_REPO /scylladb-cpp-src \
    && cd /scylladb-cpp-src \
    && mkdir build \
    && cd build \
    && cmake -G Ninja \
    -DCASS_CPP_STANDARD=17 \
    -DCASS_BUILD_STATIC=ON \
    -DCASS_BUILD_SHARED=ON \
    -DCASS_USE_STD_ATOMIC=ON \
    -DCASS_USE_STATIC_LIBS=ON \
    -DCASS_USE_TIMERFD=ON \
    -DCASS_USE_LIBSSH2=ON \
    -DCASS_USE_ZLIB=ON \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_EXPORT_COMPILE_COMMANDS="OFF" \
    -DCMAKE_INSTALL_PREFIX="/scylladb-cpp" \
    -DCMAKE_BUILD_TYPE="$SCYLLADB_DRIVER_BUILD_TYPE" .. \
    && ninja install \
    && cd ../ && rm -rf build

FROM base AS cassandra_cpp_driver_build

ARG CASSANDRA_DRIVER_BUILD_TYPE="RelWithInfo"
ARG CASSANDRA_DRIVER_REPO="https://github.com/datastax/cpp-driver.git"

ENV CXXFLAGS="-fPIC -Wno-error=redundant-move"
ENV LDFLAGS="-flto -g"

RUN git clone --depth 1 $CASSANDRA_DRIVER_REPO /cassandra-cpp-src \
    && cd /cassandra-cpp-src \
    && mkdir build \
    && cd build \
    && cmake -G Ninja \
    -DCASS_CPP_STANDARD=17 \
    -DCASS_BUILD_STATIC=ON \
    -DCASS_BUILD_SHARED=ON \
    -DCASS_USE_STD_ATOMIC=ON \
    -DCASS_USE_TIMERFD=ON \
    -DCASS_USE_LIBSSH2=ON \
    -DCASS_USE_STATIC_LIBS=ON \
    -DCASS_USE_ZLIB=ON \
    -DCASS_BUILD_TESTS=OFF \
    -DCASS_BUILD_EXAMPLES=OFF \
    -DCASS_BUILD_UNIT_TESTS=OFF \
    -DCASS_BUILD_INTEGRATION_TESTS=OFF \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_EXPORT_COMPILE_COMMANDS="OFF" \
    -DCMAKE_INSTALL_PREFIX="/cassandra-cpp" \
    -DCMAKE_BUILD_TYPE="$CASSANDRA_DRIVER_BUILD_TYPE" \
    .. \
    && ninja install \
    && cd ../ && rm -rf build

FROM base AS final

COPY --from=cassandra_cpp_driver_build /cassandra-cpp /cassandra-cpp
COPY --from=cassandra_cpp_driver_build /cassandra-cpp-src /cassandra-cpp-src

COPY --from=scylladb_cpp_driver_build /scylladb-cpp /scylladb-cpp
COPY --from=scylladb_cpp_driver_build /scylladb-cpp-src /scylladb-cpp-src
COPY --from=php_build /php /php

ENV PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/cassandra-cpp/lib/pkgconfig:/scylladb-cpp/lib/pkgconfig"
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/scylladb-cpp/lib:/cassandra-cpp/lib"
ENV PATH="$PATH:/root/.local/bin:/php/bin"

RUN apt-get update \
    && apt-get upgrade -y \
    && CODENAME=$(grep VERSION_CODENAME /etc/os-release | cut -d= -f2); [ "$CODENAME" = "focal" ] && apt-get install -y mlocate python3.8-venv \
    || apt-get install -y plocate \
    && apt-get install -y \
    pipx \
    gdb \
    python3 \
    python3-pip \
    unzip \
    clang-format \
    lldb \
    libonig5 \
    libgmp-dev \
    vim \
    ca-certificates \
    procps \
    curl \
    wget \
    && pipx install cqlsh \
    && updatedb