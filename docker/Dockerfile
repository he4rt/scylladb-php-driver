FROM ubuntu:24.04 as base

ARG PHP_VERSION=8.3.8
ARG PHP_ZTS="no"

ENV PATH="$PATH:$HOME/.local/bin:$HOME/php/bin"
ENV LD_LIBRARY_PATH="/lib/x86_64-linux-gnu/:/usr/local/lib/:/usr/lib:/lib:/lib64"

RUN apt-get update -y \
    && apt-get upgrade -y \
    && apt-get install -y \
        autoconf \
        pkg-config \
        sudo \
        wget \
        git \
        gcc \
        g++ \
        gdb \
        python3 \
        python3-pip \
        unzip \
        plocate \
        build-essential \
        ninja-build \
        libasan8 \
        libssl-dev \
        libubsan1 \
        cmake \
        pipx \
        libssl-dev \
        bison \
        re2c \
        libxml2-dev \
        libicu-dev \
        libsqlite3-dev \
        zlib1g-dev \
        libcurl4-openssl-dev \
        libreadline-dev \
        libffi-dev \
        libonig-dev \
        libsodium-dev \
        libgmp-dev \
        libasan8 \
        libubsan1 \
        libzip-dev \
    && pipx install cqlsh

COPY ./scripts /tmp/scripts

WORKDIR /tmp

RUN ./scripts/compile-php.sh -v $PHP_VERSION -o $HOME -s -d no -zts $PHP_ZTS \
    && $HOME/php/bin/php -r "copy('https://getcomposer.org/installer', 'composer-setup.php');" \
    && $HOME/php/bin/php composer-setup.php --install-dir=/bin --filename=composer \
    && $HOME/php/bin/php -r "unlink('composer-setup.php');" \
    && rm -rf /tmp/scripts

RUN git clone --depth 1 -b v1.46.0 https://github.com/libuv/libuv.git \
    && cd libuv \
    && mkdir build \
    && cd build \
    && cmake -DBUILD_TESTING=OFF -DBUILD_BENCHMARKS=OFF -DLIBUV_BUILD_SHARED=ON CMAKE_C_FLAGS="-fPIC" -DCMAKE_BUILD_TYPE="RelWithInfo" -G Ninja .. \
    && ninja install \
    && cd ../.. \
    && rm -rf libuv

RUN git clone --depth 1 https://github.com/scylladb/cpp-driver.git scylladb-driver \
    && cd scylladb-driver \
    && mkdir build \
    && cd build \
    && cmake -DCASS_CPP_STANDARD=17 -DCASS_BUILD_STATIC=ON -DCASS_BUILD_SHARED=ON -DCASS_USE_STD_ATOMIC=ON -DCASS_USE_TIMERFD=ON -DCASS_USE_LIBSSH2=ON -DCASS_USE_ZLIB=ON CMAKE_C_FLAGS="-fPIC" -DCMAKE_CXX_FLAGS="-fPIC -Wno-error=redundant-move" -DCMAKE_BUILD_TYPE="RelWithInfo" -G Ninja .. \
    && ninja install \
    && cd ../.. \
    && rm -rf scylladb-driver

RUN git clone --depth 1 https://github.com/datastax/cpp-driver.git cassandra-driver \
    && cd cassandra-driver \
    && mkdir build \
    && cd build \
    && cmake -DCASS_CPP_STANDARD=17 -DCASS_BUILD_STATIC=ON -DCASS_BUILD_SHARED=ON -DCASS_USE_STD_ATOMIC=ON -DCASS_USE_TIMERFD=ON -DCASS_USE_LIBSSH2=ON -DCASS_USE_ZLIB=ON CMAKE_C_FLAGS="-fPIC" -DCMAKE_CXX_FLAGS="-fPIC -Wno-error=redundant-move" -DCMAKE_BUILD_TYPE="RelWithInfo" -G Ninja .. \
    && ninja install \
    && cd ../.. \
    && rm -rf cassandra-driver
