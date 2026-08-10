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

RE2C_MIN_VERSION="1.0.3"
RE2C_SOURCE_VERSION="3.1"

# EPEL 8 stops at re2c 0.14.3, and PHP refuses to generate its lexers with it.
# No newer package exists for that distribution, so build the tool.
install_re2c_from_source() {
    # `|| true` because the whole point is that re2c may be absent, and
    # pipefail turns the resulting 127 into a script-ending failure.
    local have
    have="$(re2c --version 2>/dev/null | awk '{print $2}' || true)"

    if [[ -n "$have" ]] \
       && [[ "$(printf '%s\n%s\n' "$have" "$RE2C_MIN_VERSION" | sort -V | head -1)" == "$RE2C_MIN_VERSION" ]]; then
        echo "==> re2c $have is new enough"
        return 0
    fi

    echo "==> Building re2c $RE2C_SOURCE_VERSION from source (found: ${have:-none})"
    local src="/tmp/re2c-${RE2C_SOURCE_VERSION}"
    rm -rf "$src" "${src}.tar.xz"
    curl -fL -o "${src}.tar.xz" \
        "https://github.com/skvadrik/re2c/releases/download/${RE2C_SOURCE_VERSION}/re2c-${RE2C_SOURCE_VERSION}.tar.xz"
    mkdir -p "$src"
    tar -C "$src" --strip-components=1 -xJf "${src}.tar.xz"
    (
        cd "$src"
        ./configure --prefix=/usr/local
        make -j"$(nproc)"
        make install
    )
    rm -rf "$src" "${src}.tar.xz"
    hash -r
}

install_system_deps() {
    case "$(uname -s)" in
        Darwin)
            echo "==> Installing build deps (macOS)"
            command -v brew >/dev/null 2>&1 || die "Homebrew not found. Install from https://brew.sh"
            brew install re2c bison openssl zlib curl readline libffi oniguruma \
                libsodium gmp autoconf pkg-config
            ;;
        Linux)
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
                almalinux|rhel|centos|rocky)
                    # manylinux_2_28 targets this family. oniguruma-devel,
                    # libsodium-devel and jq come from EPEL, which the manylinux
                    # image enables already.
                    echo "==> Installing build deps (RHEL family)"
                    dnf install -y --setopt=install_weak_deps=False \
                        bison jq make gcc gcc-c++ pkgconf-pkg-config xz \
                        openssl-devel sqlite-devel zlib-devel libcurl-devel \
                        readline-devel libffi-devel oniguruma-devel libxml2-devel \
                        libsodium-devel gmp-devel libicu-devel libzip-devel
                    install_re2c_from_source
                    ;;
                *)
                    echo "WARN: Unknown distro '${ID:-}', skipping automatic dep install" >&2
                    ;;
            esac
            ;;
    esac
}

# php.net serves the release index without authentication and without a rate
# limit. The GitHub tags API is the fallback: it allows 60 anonymous calls per
# hour per IP, which a release matrix exhausts.
fetch_latest_php_version() {
    local version
    version="$(curl -sf "https://www.php.net/releases/index.php?json&version=${PHP_VERSION}&max=1" \
        | jq -r 'keys[]? // empty' \
        | grep -E "^${PHP_VERSION}\.[0-9]+$" \
        | sort -V | tail -n 1 || true)"

    if [[ -n "$version" ]]; then
        echo "$version"
        return 0
    fi

    echo "WARN: php.net release index unavailable, falling back to the GitHub API" >&2

    local page
    local -a auth=()
    if [[ -n "${GITHUB_TOKEN:-}" ]]; then
        auth=(-H "Authorization: Bearer ${GITHUB_TOKEN}")
    fi

    for page in 1 2 3; do
        curl -sf "${auth[@]}" \
            "https://api.github.com/repos/php/php-src/tags?page=${page}&per_page=100" \
            | jq -r '.[].name' || true
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

# Source selection, in order of precedence:
#   PHP_SRC_BRANCH=master      → branch tip, for a version with no tag yet
#   PHP_FULL_VERSION=8.6.0alpha3 → an exact tag, including pre-releases
#   (neither)                  → newest stable tag for -v <minor>
# Tag lookup only matches stable releases, hence the two overrides.
if [[ -n "${PHP_SRC_BRANCH:-}" ]]; then
    FULL_PHP_VERSION="$PHP_SRC_BRANCH"
    TARBALL_NAME="branch-${PHP_SRC_BRANCH//\//-}"
    TARBALL_URL="https://github.com/php/php-src/archive/refs/heads/${PHP_SRC_BRANCH}.tar.gz"
    SRC_DIR_NAME="php-src-${PHP_SRC_BRANCH//\//-}"
else
    FULL_PHP_VERSION="${PHP_FULL_VERSION:-$(fetch_latest_php_version)}"
    [[ -n "$FULL_PHP_VERSION" ]] || die "Could not resolve PHP $PHP_VERSION version from GitHub API"
    TARBALL_NAME="php-${FULL_PHP_VERSION}"
    TARBALL_URL="https://github.com/php/php-src/archive/refs/tags/php-${FULL_PHP_VERSION}.tar.gz"
    SRC_DIR_NAME="php-src-php-${FULL_PHP_VERSION}"
fi

OUTPUT_PATH="$(build_output_path)"
echo "==> Building PHP $FULL_PHP_VERSION → $OUTPUT_PATH"

TARBALL_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/php-src"
mkdir -p "$TARBALL_DIR"
TARBALL="$TARBALL_DIR/${TARBALL_NAME}.tar.gz"

mkdir -p "$OUTPUT_PATH"

# A branch tarball moves, so never reuse a cached copy of one.
if [[ -n "${PHP_SRC_BRANCH:-}" || ! -f "$TARBALL" ]]; then
    echo "==> Downloading $TARBALL_URL"
    curl -fL -o "${TARBALL}.tmp" "$TARBALL_URL" && mv "${TARBALL}.tmp" "$TARBALL"
fi

echo "==> Extracting sources"
rm -rf "$OUTPUT_PATH/src"
tar -C "$OUTPUT_PATH" -xzf "$TARBALL"
mv "$OUTPUT_PATH/$SRC_DIR_NAME" "$OUTPUT_PATH/src"

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
)

# PHP 8.6 renamed --with-pic to --enable-pic; the old spelling now hard-errors.
if [[ "$(printf '%s\n8.6\n' "$PHP_VERSION" | sort -V | head -1)" == "8.6" ]]; then
    CONFIG_ARGS+=(--enable-pic)
else
    CONFIG_ARGS+=(--with-pic)
fi

if [[ "$(uname -s)" == "Darwin" ]] && command -v brew >/dev/null 2>&1; then
    BREW_PREFIX="$(brew --prefix)"
    [[ -d "$BREW_PREFIX/opt/libiconv" ]] && CONFIG_ARGS+=(--with-iconv="$BREW_PREFIX/opt/libiconv")
    [[ -d "$BREW_PREFIX/opt/bison/bin" ]] && export PATH="$BREW_PREFIX/opt/bison/bin:$PATH"
    if [[ -d "$BREW_PREFIX/opt/openssl@3" ]]; then
        export PKG_CONFIG_PATH="$BREW_PREFIX/opt/openssl@3/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
    fi
fi

[[ "$PHP_THREAD_MODEL" == "zts" ]] && CONFIG_ARGS+=(--enable-zts)
[[ "$ENABLE_SANITIZERS" == "yes" ]] && CONFIG_ARGS+=(--enable-address-sanitizer --enable-undefined-sanitizer)

NPROC="$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)"

if [[ "$ENABLE_DEBUG" == "yes" ]]; then
    CONFIG_ARGS+=(--enable-debug)
    echo "==> Configuring (debug)"
    if [[ "$(uname -s)" == "Darwin" ]]; then
        # Apple clang: no -ggdb/-gdwarf-4; macOS uses DWARF natively
        ./configure \
            CFLAGS="-g -O0 -fno-omit-frame-pointer" \
            CXXFLAGS="-g -O0 -fno-omit-frame-pointer" \
            "${CONFIG_ARGS[@]}"
    else
        ./configure \
            CFLAGS="-g -ggdb -g3 -gdwarf-4 -fno-omit-frame-pointer" \
            CXXFLAGS="-g -ggdb -g3 -gdwarf-4 -fno-omit-frame-pointer" \
            "${CONFIG_ARGS[@]}"
    fi
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
