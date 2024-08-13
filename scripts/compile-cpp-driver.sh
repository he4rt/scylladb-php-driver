SCYLLA_OR_CASSANDRA=$1
CURRENT_DIR="$(pwd)"

if [ -z "$SCYLLA_OR_CASSANDRA" ]; then
  SCYLLA_OR_CASSANDRA="scylladb"
fi

if [ "$SCYLLA_OR_CASSANDRA" = "scylladb" ]; then
  GIT_REPO="https://github.com/scylladb/cpp-driver.git"
  GIT_OUTPUT="/opt/scylladb-driver"
else
  GIT_REPO="https://github.com/datastax/cpp-driver.git"
  GIT_OUTPUT="/opt/cassandra-driver"
fi

git clone --depth 1 "$GIT_REPO" "$GIT_OUTPUT"

cd "$GIT_OUTPUT" || exit 1

mkdir build || exit 1

cd build || exit 1

CFLAGS="-fPIC" CXXFLAGS="-fPIC -Wno-error=redundant-move" LDFLAGS="-flto" cmake -G Ninja \
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
  -DCMAKE_BUILD_TYPE="RelWithInfo" \
  ..

CFLAGS="-fPIC" CXXFLAGS="-fPIC -Wno-error=redundant-move" LDFLAGS="-flto" ninja install

cd .. || exit

rm -rf build

cd "$CURRENT_DIR" || exit 1
