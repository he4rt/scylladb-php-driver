#!/usr/bin/env bash
# -*- coding: utf-8 -*-

print_usage() {
  echo ""
  echo "Usage: compile-php.sh [OPTION] [ARG]"
  echo "-v ARG php version"
  echo "-o ARG output path, default: <project_root>/third-party/php"
  echo "-z Use ZTS"
  echo "-d Compile in debug mode"
  echo "-k keep PHP source code"
  echo "-a compile PHP without version suffix"
  echo "-s Use Memory and Undefined Sanitizers"
  echo "----------"
  echo "Example: compiling PHP 8.2 in debug mode with Thread Safety"
  echo "./compile-php.sh -v 8.2 -s yes -d yes -z yes"
  echo ""
}

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PHP_VERSION="8.4"
WITH_VERSION="yes"
PHP_THREAD_MODEL="nts"
ENABLE_DEBUG="no"
OUTPUT="$PROJECT_ROOT/third-party/php"
ENABLE_SANITIZERS="no"

while getopts "v:o:z:s:d:a" option; do
  case "$option" in
  "v") PHP_VERSION="$OPTARG" ;;
  "z") PHP_THREAD_MODEL="$OPTARG" ;;
  "o") OUTPUT="$OPTARG" ;;
  "d") ENABLE_DEBUG="$OPTARG" ;;
  "s") ENABLE_SANITIZERS="$OPTARG" ;;
  "a") WITH_VERSION="no" ;;
  *) print_usage ;;
  esac
done

if [[ -z "$PHP_VERSION" ]]; then
  print_usage
  exit 1
fi

fetch_versions() {
  local page=$1

  curl -s "https://api.github.com/repos/php/php-src/tags?page=$page&per_page=100" | jq -r '.[].name'
}

fetch_latest_php_version() {
  (fetch_versions "1" && fetch_versions "2" && fetch_versions "3") |
    grep -E "^php-$PHP_VERSION\.[0-9]+$" |
    sed 's/^php-//' |
    sort -V |
    tail -n 1
}

is_linux() {
  local value

  value="$(uname -s)"

  if [ "$value" = "Linux" ]; then
    return 0
  fi

  return 1
}

is_macos() {
  local value

  value="$(uname -s)"

  if [ "$value" = "Darwin" ]; then
    return 0
  fi

  return 1
}

get_cpu_count() {
  if is_macos; then
    sysctl -n hw.ncpu
  else
    nproc
  fi
}

install_deps() {
  if is_linux; then
    . /etc/os-release

    if [[ "$NAME" == "Fedora Linux" ]]; then
      dnf install \
        re2c \
        cmake \
        gcc \
        ninja-build \
        openssl-devel \
        libubsan \
        libasan \
        sqlite-devel \
        zlib-devel \
        libcurl-devel \
        readline-devel \
        libffi-devel \
        oniguruma-devel \
        libxml2-devel \
        libsodium-devel \
        gmp-devel -y || exit 1
    fi

    if [[ "$NAME" == "Ubuntu" ]]; then
      apt-get install \
        pkg-config \
        build-essential \
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
        libubsan1 \
        libzip-dev -y || exit 1
    fi
  elif is_macos; then
    if ! command -v brew &> /dev/null; then
      echo "Homebrew is required but not installed. Please install it from https://brew.sh"
      exit 1
    fi

    brew install \
      re2c \
      cmake \
      ninja \
      bison \
      openssl \
      sqlite \
      zlib \
      curl \
      readline \
      libffi \
      oniguruma \
      libxml2 \
      icu4c \
      libsodium \
      gmp \
      libzip || exit 1
  fi
}

compile_php() {
  local config=(
    --enable-opcache
    --enable-rtld-now
    --with-openssl
    --with-zlib
    --with-curl
    --enable-pcntl
    --with-pear
    --enable-sockets
    --enable-mbstring
    --disable-short-tags
    --with-pic
  )

  # Add iconv for macOS
  if is_macos; then
    config+=("--with-iconv=$(brew --prefix)/opt/libiconv")
  fi

  local FULL_PHP_VERSION
  FULL_PHP_VERSION="$(fetch_latest_php_version)"

  local OUTPUT_PATH="$OUTPUT"

  if [[ "$WITH_VERSION" == "yes" ]]; then
    OUTPUT_PATH="$OUTPUT_PATH/$PHP_VERSION"
    if [[ "$ENABLE_DEBUG" == "yes" ]]; then
      OUTPUT_PATH="$OUTPUT_PATH-debug"
      config+=("--enable-debug")
    else
      OUTPUT_PATH="$OUTPUT_PATH-release"
    fi

    if [[ "$PHP_THREAD_MODEL" == "zts" ]]; then
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

  if [ ! -f "php-$FULL_PHP_VERSION.tar.gz" ]; then
    wget -O "php-$FULL_PHP_VERSION.tar.gz" "https://github.com/php/php-src/archive/refs/tags/php-$FULL_PHP_VERSION.tar.gz" || exit 1
  fi

  tar -C "$OUTPUT_PATH" -xzf "php-$FULL_PHP_VERSION.tar.gz" || exit 1
  mv "$OUTPUT_PATH/php-src-php-$FULL_PHP_VERSION" "$OUTPUT_PATH/src" || exit 1

  rm -f "php-$FULL_PHP_VERSION.tar.gz" || exit 1

  pushd "$OUTPUT_PATH/src" || exit 1

  ./buildconf --force
  if [ "$ENABLE_DEBUG" = "yes" ]; then
    ./configure \
      CFLAGS="-g -ggdb -g3 -gdwarf-5 -fno-omit-frame-pointer" \
      CXXFLAGS="-g -ggdb -g3 -gdwarf-5 -fno-omit-frame-pointer" \
      --prefix="$OUTPUT_PATH" \
      "${config[@]}" || exit 1
  else
    ./configure CFLAGS="-O2" CXXFLAGS="-O2" --prefix="$OUTPUT_PATH" "${config[@]}" || exit 1
  fi
  make "-j$(get_cpu_count)" || exit 1
  make install || exit 1

  popd || exit 1
}

compile_php
