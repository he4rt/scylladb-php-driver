FROM ubuntu:22.04 as base

ARG PHP_VERSION=8.3.0
ARG PHP_ZTS="no"
ARG CASSANDRA_DRIVER_VERSION=2.17.1

ENV PATH="$PATH:$HOME/.local/bin:$HOME/php/bin:/usr/local/bin:/root/php/bin"
ENV LD_LIBRARY_PATH="/usr/lib:/lib:/lib64:/lib/x86_64-linux-gnu:/usr/local/lib:/usr/lib:/lib:/usr/local/lib/x86_64-linux-gnu"

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
    mlocate \
    build-essential \
    ninja-build \
    libasan8 \
    libubsan1 \
    && pip3 install cmake cqlsh \
    && apt-get clean

COPY ./scripts /tmp/scripts

WORKDIR /tmp

RUN ./scripts/compile-php.sh -v $PHP_VERSION -o $HOME -s -d no -zts $PHP_ZTS \
    && $HOME/php/bin/php -r "copy('https://getcomposer.org/installer', 'composer-setup.php');" \
    && $HOME/php/bin/php composer-setup.php --install-dir=/bin --filename=composer \
    && $HOME/php/bin/php -r "unlink('composer-setup.php');" \
    && rm -rf /tmp/scripts

RUN git clone --depth 1 -b v1.47.0 https://github.com/libuv/libuv.git \
    && cd libuv \
    && mkdir build \
    && cd build \
    && cmake -DBUILD_TESTING=OFF -DBUILD_BENCHMARKS=OFF -DLIBUV_BUILD_SHARED=ON CMAKE_C_FLAGS="-fPIC" -DCMAKE_BUILD_TYPE="RelWithInfo" -G Ninja .. \
    && ninja install \
    && cd ../.. \
    && rm -rf libuv

RUN git clone --depth 1 https://github.com/datastax/cpp-driver.git cassandra-driver \
    && cd cassandra-driver \
    && git checkout -b v$CASSANDRA_DRIVER_VERSION tags/$CASSANDRA_DRIVER_VERSION \
    && mkdir build \
    && cd build \
    && cmake -DCASS_CPP_STANDARD=17 -DCASS_BUILD_STATIC=ON -DCASS_BUILD_SHARED=ON -DCASS_USE_STD_ATOMIC=ON -DCASS_USE_TIMERFD=ON -DCASS_USE_LIBSSH2=ON -DCASS_USE_ZLIB=ON CMAKE_C_FLAGS="-fPIC" -DCMAKE_CXX_FLAGS="-fPIC -Wno-error=redundant-move" -DCMAKE_BUILD_TYPE="RelWithInfo" -G Ninja .. \
    && ninja install \
    && cd ../.. \
    && rm -rf cassandra-driver
