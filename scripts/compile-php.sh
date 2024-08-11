#!/usr/bin/env bash
# -*- coding: utf-8 -*-

print_usage() {
  echo ""
  echo "Usage: compile-php.sh [OPTION] [ARG]"
  echo "-v ARG php version"
  echo "-o ARG output path, default: $(pwd)"
  echo "-z Use ZTS"
  echo "-d Compile in debug mode"
  echo "-k keep PHP source code"
  echo "-a compile PHP without version suffix"
  echo "-s Use Memory and Undefined Sanitizers"
  echo "----------"
  echo "Example: compiling PHP 8.2.7 in debug mode with Thread Safety"
  echo "./compile-php.sh -v 8.2.7 -s -d -z"
  echo ""
}

which_linux() {
  local val=$(grep "NAME=\"$1\"" "/etc/os-release")

  if [ "$val" = "NAME=$1" ]; then
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

is_macos() {
  local value

  value=$(uname -s)

  if [ "$value" = "Darwin" ]; then
    return 0
  fi

  return 1
}

install_macos_dep() {
  local package_path
  local path
  local PKG_CONFIG=""
  local PATH_EXPORT=""

  for package in "$@"; do
    output=$(brew install "$package")

    package_path=$(perl -lne 'print $1 if /export PKG_CONFIG_PATH=\"(.*)\"/;' <"$output")
    path=$(perl -lne 'print $1 if /export PATH=\"(.*):$PATH\"/;' <"$output")
    if [ -n "$package_path" ]; then
      PKG_CONFIG="$PKG_CONFIG:$package_path"
    fi
    if [ -n "$path" ]; then
      PATH_EXPORT="$PATH_EXPORT$path:"
    fi
  done

  if [ -n "$PKG_CONFIG" ]; then
    echo "export PKG_CONFIG_PATH=\"\$PKG_CONFIG_PATH$PKG_CONFIG\"" >>~/.profile
  fi

  if [ -n "$PATH_EXPORT" ]; then
    echo "export PATH=\"$PATH_EXPORT\$PATH\"" >>~/.profile
  fi

}

install_deps() {
  if is_macos; then
    install_macos_dep \
      pkg-config \
      bison \
      re2c \
      libxml2 \
      sqlite3 \
      zlib-ng \
      readline \
      libiconv \
      libffi
  fi

  if is_linux; then
    if which_linux "Fedora Linux"; then
      sudo dnf install \
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

    if which_linux "Ubuntu"; then
      sudo apt-get install \
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
        libasan8 \
        libubsan1 \
        libzip-dev -y || exit 1
    fi

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
    --enable-phpdbg-debug
    --enable-rtld-now
    --with-openssl
    --with-zlib
    --with-curl
    --with-ffi
    --enable-pcntl
    --with-pear
    --enable-sockets
    --with-pic
    --enable-mbstring
    --with-sqlite3
    --enable-calendar
  )

  local OUTPUT_PATH="$OUTPUT"

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

  if is_macos; then
    config+=("--with-iconv=/opt/homebrew/opt/libiconv")
  fi

  rm -rf "$OUTPUT_PATH" || exit 1
  mkdir -p "$OUTPUT_PATH" || exit 1

  if [ ! -f "php-$PHP_VERSION.tar.gz" ]; then
    wget -O "php-$PHP_VERSION.tar.gz" "https://github.com/php/php-src/archive/refs/tags/php-$PHP_VERSION.tar.gz" || exit 1
  fi

  tar -C "$OUTPUT_PATH" -xzf "php-$PHP_VERSION.tar.gz" || exit 1
  mv "$OUTPUT_PATH/php-src-php-$PHP_VERSION" "$OUTPUT_PATH/src" || exit 1

  if [[ "$KEEP_PHP_SOURCE" == "no" ]]; then
    rm -f "php-$PHP_VERSION.tar.gz" || exit 1
  fi

  pushd "$OUTPUT_PATH/src" || exit 1

  {
    ./buildconf --force
    ./configure CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" --prefix="$OUTPUT_PATH" "${config[@]}" || exit 1
    make "-j$(nproc)" || exit 1
    make install || exit 1
  }

  popd || exit 1
}

check_deps() {
  deps="wget make git gcc g++"

  for dep in $deps; do
    [ -z "$(command -v "$dep")" ] && echo "Unsatisfied dependency: $dep" && exit 1
  done
}

check_deps

while getopts "v:zo:s:d:ka" option; do
  case "$option" in
  "v") PHP_VERSION="$OPTARG" ;;
  "z") PHP_ZTS="$OPTARG" ;;
  "o") OUTPUT="$OPTARG" ;;
  "d") ENABLE_DEBUG="$OPTARG" ;;
  "k") KEEP_PHP_SOURCE="yes" ;;
  "s") ENABLE_SANITIZERS="$OPTARG" ;;
  "a") WITHOUT_VERSION="yes" ;;
  *) print_usage ;;
  esac
done

if [[ -z "$WITHOUT_VERSION" ]]; then
  WITHOUT_VERSION="no"
fi

if [[ -z "$PHP_ZTS" ]]; then
  PHP_ZTS="nts"
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

pritnf "\n\nCompiling PHP $PHP_VERSION in $OUTPUT: Debug mode: $ENABLE_DEBUG, Thread Safety: $PHP_ZTS, Sanitizers: $ENABLE_SANITIZERS\n"

compile_php "$PHP_ZTS" "$ENABLE_DEBUG" "$KEEP_PHP_SOURCE" || exit 1
