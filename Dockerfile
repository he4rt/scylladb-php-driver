ARG IMAGE=malusevd99/php-ext-dev:8.1

FROM ${IMAGE} as production

ARG CPP_DRIVER_VERSION=2.16.2
ENV EXT_CASSANDRA_VERSION=master
ENV LDFLAGS="-L/usr/local/lib"
ENV LIBS="-lssl -lz -luv -lm -lgmp -lstdc++"
ENV PHP_INI_SCAN_DIR=":/usr/local/lib/php"

ENV CASSANDRA_HOST=cassandra
ENV PHP_INI_SCAN_DIR=":/usr/local/lib/php"

RUN php -r "copy('https://getcomposer.org/installer', 'composer-setup.php');" && \
    php composer-setup.php && php -r "unlink('composer-setup.php');" \
    mv composer.phar /bin/composer

COPY ./php.ini /php.ini

RUN docker-php-source extract \
    && apt-get update -y \
    && apt-get upgrade -y \
    && apt-get install \
    cmake \
    unzip \
    build-essential \
    git \
    gcc \
    make \
    ninja-build \
    libuv1-dev \
    libssl-dev \
    libgmp-dev \
    zlib1g-dev \
    openssl \
    python3 \
    libpcre3-dev \
    apt-utils \
    openssh-client \
    gnupg2 \
    wget \
    ca-certificates \
    lsb-release \
    apt-transport-https \
    libc6 \
    libgcc1 \
    libgssapi-krb5-2 \
    libicu-dev \
    liblttng-ust0 \
    libstdc++6 \
    zlib1g -y

RUN git clone --recursive https://github.com/datastax/cpp-driver /cpp-driver \
    && cd /cpp-driver && git checkout tags/$CPP_DRIVER_VERSION -b v$CPP_DRIVER_VERSION \
    && mkdir -p build && cd build \
    && cmake \
    -G Ninja \
    -DCMAKE_CXX_FLAGS="-fPIC" \
    -DCMAKE_C_FLAGS="-fPIC" \
    -DCASS_BUILD_STATIC=ON \
    -DCASS_BUILD_SHARED=ON \
    -DCMAKE_BUILD_TYPE=RELEASE \
    -DCASS_USE_STATIC_LIBS=ON \
    -DCASS_USE_TIMERFD=ON \
    -DCMAKE_INSTALL_LIBDIR:PATH=lib \
    -DCASS_USE_ZLIB=ON .. \
    && ninja && ninja install \
    && cd / && rm -rf /cpp-driver

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs >> /tmp/install-rust \
    && chmod +x /tmp/install-rust \
    && sh /tmp/install-rust --profile default --default-toolchain stable -y \
    && git clone https://github.com/corrosion-rs/corrosion.git /tmp/corrosion \
    && cd /tmp && cmake -Scorrosion -Bbuild -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release \
    && cmake --install build --config Release

RUN php-config --ini-path | xargs echo | xargs cp /php.ini

SHELL ["/bin/bash", "-c"]

CMD ["bash"]

FROM ${IMAGE} as dev
ARG CPP_DRIVER_VERSION=2.16.2
ENV EXT_CASSANDRA_VERSION=master
ENV LDFLAGS="-L/usr/local/lib"
ENV LIBS="-lssl -lz -luv -lm -lgmp -lstdc++"
ENV PHP_INI_SCAN_DIR=":/usr/local/lib/php"

ENV JAVA_HOME=/usr/lib/jvm/default-java
ENV CASSANDRA_HOST=cassandra
ENV PHP_INI_SCAN_DIR=":/usr/local/lib/php"

RUN php -r "copy('https://getcomposer.org/installer', 'composer-setup.php');" && \
    php composer-setup.php && php -r "unlink('composer-setup.php');" \
    mv composer.phar /bin/composer

COPY --from=mlocati/php-extension-installer /usr/bin/install-php-extensions /usr/local/bin

RUN docker-php-source extract \
    && apt-get update -y \
    && apt-get install \
    cmake \
    unzip \
    mlocate \
    build-essential \
    git \
    gcc \
    make \
    ninja-build \
    libuv1-dev \
    libssl-dev \
    libgmp-dev \
    zlib1g-dev \
    openssl \
    python3 \
    python3-pip \
    python3-distutils \
    cppcheck \
    flawfinder \
    psutils \
    iputils-ping \
    libpcre3-dev \
    apt-utils \
    openssh-client \
    gnupg2 \
    dirmngr \
    iproute2 \
    procps \
    lsof \
    htop \
    net-tools \
    psmisc \
    curl \
    wget \
    rsync \
    ca-certificates \
    unzip \
    zip \
    nano \
    vim-tiny \
    less \
    jq \
    lsb-release \
    apt-transport-https \
    dialog \
    libc6 \
    libgcc1 \
    libkrb5-3 \
    libgssapi-krb5-2 \
    libicu-dev \
    liblttng-ust0 \
    libstdc++6 \
    zlib1g \
    locales \
    sudo \
    ncdu \
    man-db \
    strace \
    manpages \
    manpages-dev \
    init-system-helpers \
    default-jdk -y

RUN git clone --recursive https://github.com/datastax/cpp-driver /cpp-driver \
    && cd /cpp-driver && git checkout tags/$CPP_DRIVER_VERSION -b v$CPP_DRIVER_VERSION \
    && mkdir -p build && cd build \
    && cmake \
    -G Ninja \
    -DCMAKE_CXX_FLAGS="-fPIC" \
    -DCASS_BUILD_STATIC=ON \
    -DCASS_BUILD_SHARED=ON \
    -DCMAKE_BUILD_TYPE=RELEASE \
    -DCMAKE_INSTALL_LIBDIR:PATH=lib \
    -DCASS_USE_ZLIB=ON .. \
    && ninja && ninja install \
    && cd / && rm -rf /cpp-driver

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs >> /tmp/install-rust \
    && chmod +x /tmp/install-rust \
    && sh /tmp/install-rust --profile default --default-toolchain stable -y \
    && git clone https://github.com/corrosion-rs/corrosion.git /tmp/corrosion \
    && cd /tmp && cmake -Scorrosion -Bbuild -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release \
    && cmake --install build --config Release \
    && install-php-extensions intl zip pcntl gmp ast xdebug \
    && mkdir -p /ccm

RUN git clone https://github.com/riptano/ccm.git /ccm \
    && cd /ccm && python3 setup.py install \
    && cd / && rm -rf /ccm \
    && pip3 install lizard \
    && pip3 install six \
    && pip3 install psutil

COPY ./php.ini /php.ini
COPY ./setup.sh /setup.sh

RUN chmod +x /setup.sh && bash /setup.sh && php-config --ini-path | xargs echo | xargs cp /php.ini
SHELL ["/bin/bash", "-c"]
CMD ["bash"]
