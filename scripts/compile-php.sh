#!/usr/bin/env bash
set -euo pipefail

PHP_VERSION="8.4"
OUTPUT="$(pwd)/php"
PHP_THREAD_MODEL="nts"
ENABLE_DEBUG="no"
ENABLE_SANITIZERS="no"
WITH_VERSION="yes"
KEEP_SRC="no"

print_usage() {
    cat <<EOF

Usage: $(basename "$0") [OPTIONS]

Options:
  -v VERSION   PHP minor version to build (default: 8.4)
  -o PATH      Output directory (default: \$(pwd)/php)
  -z           Enable ZTS (thread-safe build)
  -d           Compile in debug mode
  -s           Enable AddressSanitizer + UndefinedSanitizer
  -k           Keep PHP source after install
  -a           Install without version suffix in output path
  -h           Show this help

Examples:
  $(basename "$0") -v 8.4
  $(basename "$0") -v 8.3 -d -z -o /opt/php
  $(basename "$0") -v 8.5 -s -d

EOF
}

die() { echo "ERROR: $*" >&2; exit 1; }

while getopts "v:o:zsdkah" opt; do
    case "$opt" in
        v) PHP_VERSION="$OPTARG" ;;
        o) OUTPUT="$OPTARG" ;;
        z) PHP_THREAD_MODEL="zts" ;;
        d) ENABLE_DEBUG="yes" ;;
        s) ENABLE_SANITIZERS="yes" ;;
        k) KEEP_SRC="yes" ;;
        a) WITH_VERSION="no" ;;
        h) print_usage; exit 0 ;;
        *) print_usage; exit 1 ;;
    esac
done

[[ -n "$PHP_VERSION" ]] || die "PHP version is required (-v)"

install_system_deps() {
    if [[ "$(uname -s)" != "Linux" ]]; then return; fi
    # shellcheck source=/dev/null
    . /etc/os-release 2>/dev/null || return

    case "${ID:-}" in
        fedora)
            echo "==> Installing build deps (Fedora)"
            dnf install -y re2c cmake gcc ninja-build openssl-devel \
                libubsan libasan sqlite-devel zlib-devel libcurl-devel \
                readline-devel libffi-devel oniguruma-devel libxml2-devel \
                libsodium-devel gmp-devel bison
            ;;
        ubuntu|debian)
            echo "==> Installing build deps (Ubuntu/Debian)"
            apt-get install -y pkg-config build-essential libssl-dev bison re2c \
                libxml2-dev libicu-dev libsqlite3-dev zlib1g-dev libcurl4-openssl-dev \
                libreadline-dev libffi-dev libonig-dev libsodium-dev libgmp-dev \
                libubsan1 libzip-dev
            ;;
        *)
            echo "WARN: Unknown distro '${ID:-}', skipping automatic dep install" >&2
            ;;
    esac
}

fetch_latest_php_version() {
    local page
    for page in 1 2 3; do
        curl -sf "https://api.github.com/repos/php/php-src/tags?page=${page}&per_page=100" \
            | jq -r '.[].name'
    done \
        | grep -E "^php-${PHP_VERSION}\.[0-9]+$" \
        | sed 's/^php-//' \
        | sort -V \
        | tail -n 1
}

build_output_path() {
    local path="$OUTPUT"
    if [[ "$WITH_VERSION" == "yes" ]]; then
        path="${path}/${PHP_VERSION}"
        [[ "$ENABLE_DEBUG" == "yes" ]] && path="${path}-debug" || path="${path}-release"
        [[ "$PHP_THREAD_MODEL" == "zts" ]] && path="${path}-zts" || path="${path}-nts"
    fi
    echo "$path"
}

install_system_deps

command -v curl  >/dev/null 2>&1 || die "curl not found in PATH"
command -v jq    >/dev/null 2>&1 || die "jq not found in PATH"
command -v make  >/dev/null 2>&1 || die "make not found in PATH"
command -v bison >/dev/null 2>&1 || die "bison not found in PATH"
command -v re2c  >/dev/null 2>&1 || die "re2c not found in PATH"

FULL_PHP_VERSION="$(fetch_latest_php_version)"
[[ -n "$FULL_PHP_VERSION" ]] || die "Could not resolve PHP $PHP_VERSION version from GitHub API"

OUTPUT_PATH="$(build_output_path)"
echo "==> Building PHP $FULL_PHP_VERSION → $OUTPUT_PATH"

TARBALL_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/php-src"
mkdir -p "$TARBALL_DIR"
TARBALL="$TARBALL_DIR/php-${FULL_PHP_VERSION}.tar.gz"
TARBALL_URL="https://github.com/php/php-src/archive/refs/tags/php-${FULL_PHP_VERSION}.tar.gz"

mkdir -p "$OUTPUT_PATH"

if [[ ! -f "$TARBALL" ]]; then
    echo "==> Downloading $TARBALL_URL"
    curl -fL -o "${TARBALL}.tmp" "$TARBALL_URL" && mv "${TARBALL}.tmp" "$TARBALL"
fi

echo "==> Extracting sources"
tar -C "$OUTPUT_PATH" -xzf "$TARBALL"
mv "$OUTPUT_PATH/php-src-php-${FULL_PHP_VERSION}" "$OUTPUT_PATH/src"

[[ "$KEEP_SRC" == "no" ]] && rm -f "$TARBALL"

pushd "$OUTPUT_PATH/src"

./buildconf --force

CONFIG_ARGS=(
    --prefix="$OUTPUT_PATH"
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

[[ "$PHP_THREAD_MODEL" == "zts" ]] && CONFIG_ARGS+=(--enable-zts)
[[ "$ENABLE_SANITIZERS" == "yes" ]] && CONFIG_ARGS+=(--enable-address-sanitizer --enable-undefined-sanitizer)

NPROC="$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)"

if [[ "$ENABLE_DEBUG" == "yes" ]]; then
    CONFIG_ARGS+=(--enable-debug)
    echo "==> Configuring (debug)"
    ./configure \
        CFLAGS="-g -ggdb -g3 -gdwarf-4 -fno-omit-frame-pointer" \
        CXXFLAGS="-g -ggdb -g3 -gdwarf-4 -fno-omit-frame-pointer" \
        "${CONFIG_ARGS[@]}"
else
    echo "==> Configuring (release)"
    ./configure CFLAGS="-O2" CXXFLAGS="-O2" "${CONFIG_ARGS[@]}"
fi

echo "==> Building (jobs=$NPROC)"
make -j"$NPROC"

echo "==> Installing to $OUTPUT_PATH"
make install

popd

if [[ "$KEEP_SRC" == "no" ]]; then
    rm -rf "$OUTPUT_PATH/src"
fi

echo "==> PHP $FULL_PHP_VERSION installed to $OUTPUT_PATH"
echo "    Binary: $OUTPUT_PATH/bin/php"
