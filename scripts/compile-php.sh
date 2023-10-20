#!/usr/bin/env bash
# -*- coding: utf-8 -*-

print_usage() {
    echo ""
    echo "Usage: compile-php.sh [OPTION] [ARG]"
    echo "-v ARG php version"
    echo "-o ARG output path, default: $(pwd)"
    echo "-z (yes|no) Use ZTS"
    echo "-d (yes|no) Compile in debug mode"
    echo "-k keep PHP source code"
    echo "-a compile PHP without version suffix"
    echo "-s Use Memory and Undefined Sanitizers"
    echo "----------"
    echo "Example: compiling PHP 8.2.7 in debug mode with Thread Safety"
    echo "./compile-php.sh -v 8.2.7 -s -d yes -z yes"
    echo ""
}

which_linux() {
    local value=$1

    if grep "NAME=\"$value\"" "/etc/os-release" >>/dev/null; then
        return 0
    fi

    return 1
}

is_linux() {
    local value

    value=$(uname -s)

    if [ "$value" = "Linux" ]; then
        return 0
    fi

    return 1
}

install_deps() {
    if which_linux "Ubuntu"; then
        sudo apt-get install \
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
            libbz2-dev \
            libsodium-dev \
            libgmp-dev \
            libasan8 \
            libubsan1 \
            libzip-dev -y >>/dev/null || exit 1
    fi

    if which_linux "Fedora Linux"; then
        # TODO: install libasan and libubsan
        sudo dnf install \
            openssl-devel \
            sqlite-devel \
            zlib-devel \
            libcurl-devel \
            readline-devel \
            libffi-devel \
            oniguruma-devel \
            bzip2-devel \
            libsodium-devel \
            gmp-devel \
            libzip-devel -y >>/dev/null || exit 1
    fi
}

compile_php() {
    local ZTS="$1"
    local WITH_DEBUG="$2"
    local KEEP_PHP_SOURCE="$3"

    local PHP_BASE_VERSION=$(echo "$PHP_VERSION" | cut -d. -f1,2)
    local config=(
        --enable-embed=static
        --enable-phpdbg
        --enable-opcache
        --disable-short-tags
        --with-openssl
        --with-zlib
        --with-curl
        --with-ffi
        --enable-pcntl
        --enable-sockets
        --with-zip
        --with-pic
        --with-readline
        --enable-mbstring
        --with-sqlite3
        --enable-fpm
        --enable-calendar
        --enable-bcmath
        --with-bz2
        --with-gmp
        --with-gettext
        --with-mysqli
        --with-sodium
    )

    local OUTPUT_PATH="$OUTPUT/php/"

    if [[ "$WITHOUT_VERSION" == "yes" ]]; then
        OUTPUT_PATH="$OUTPUT_PATH/$PHP_BASE_VERSION"
        if [[ "$WITH_DEBUG" == "yes" ]]; then
            OUTPUT_PATH="$OUTPUT_PATH-debug"
            config+=("--enable-debug")
        else
            OUTPUT_PATH="$OUTPUT_PATH-release"
        fi

        if [[ "$ZTS" == "yes" || "$ZTS" == "zts" ]]; then
            OUTPUT_PATH="$OUTPUT_PATH-zts"
            config+=("--enable-zts")
        else
            OUTPUT_PATH="$OUTPUT_PATH-nts"
        fi
    fi

    if [[ "$ENABLE_SANITIZERS" == "yes" ]]; then
        config+=("--enable-address-sanitizer" "--enable-undefined-sanitizer")
    fi

    rm -rf "$OUTPUT_PATH" || exit 1
    mkdir -p "$OUTPUT_PATH" || exit 1

    if [ ! -f "php-$PHP_VERSION.tar.gz" ]; then
        wget -O "php-$PHP_VERSION.tar.gz" "https://github.com/php/php-src/archive/refs/tags/php-$PHP_VERSION.tar.gz" >>/dev/null || exit 1
    fi

    tar -C "$OUTPUT_PATH" -xzf "php-$PHP_VERSION.tar.gz" || exit 1
    mv "$OUTPUT_PATH/php-src-php-$PHP_VERSION" "$OUTPUT_PATH/src" || exit 1

    if [[ "$KEEP_PHP_SOURCE" == "no" ]]; then
        rm -f "php-$PHP_VERSION.tar.gz" || exit 1
    fi

    pushd "$OUTPUT_PATH/src" || exit 1

    {
        ./buildconf --force >>/dev/null || exit 1
        ./configure CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" --prefix="$OUTPUT_PATH" "${config[@]}" >>/dev/null || exit 1
        make "-j$(nproc)" || exit 1
        make install || exit 1
    } >>/dev/null

    popd || exit 1
}

check_deps() {
    deps="wget make git cmake gcc g++"

    for dep in $deps; do
        [ -z "$(command -v "$dep")" ] && echo "Unsatisfied dependency: $dep" && exit 1
    done
}

check_deps

while getopts "v:z:o:sd:ka" option; do
    case "$option" in
    "v") PHP_VERSION="$OPTARG" ;;
    "z") PHP_ZTS="$OPTARG" ;;
    "o") OUTPUT="$OPTARG" ;;
    "d") ENABLE_DEBUG="$OPTARG" ;;
    "k") KEEP_PHP_SOURCE="yes" ;;
    "s") ENABLE_SANITIZERS="yes" ;;
    "a") WITHOUT_VERSION="yes" ;;
    *) print_usage ;;
    esac
done

if [[ -z "$WITHOUT_VERSION" ]]; then
    WITHOUT_VERSION="no"
fi

if [[ -z "$PHP_ZTS" ]]; then
    PHP_ZTS="no"
fi

if [[ -z "$KEEP_PHP_SOURCE" ]]; then
    KEEP_PHP_SOURCE="no"
fi

if [[ -z "$ENABLE_DEBUG" ]]; then
    ENABLE_DEBUG="no"
fi

if [[ -z "$OUTPUT" ]]; then
    OUTPUT="$(pwd)"
fi

if [[ -z "$ENABLE_SANITIZERS" ]]; then
    ENABLE_SANITIZERS="no"
fi

if [[ -z "$PHP_VERSION" ]]; then
    print_usage
    exit 1
fi

CFLAGS="-g -ggdb -g3 -gdwarf-4 -fno-omit-frame-pointer"
CXXFLAGS="-g -ggdb -g3 -gdwarf-4 -fno-omit-frame-pointer"

install_deps || exit 1

compile_php "$PHP_ZTS" "$ENABLE_DEBUG" "$KEEP_PHP_SOURCE" || exit 1
