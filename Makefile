BUILD_TYPE ?= Release

ifeq ($(BUILD_TYPE),Debug)
	BUILD_FOLDER = build-debug
else
	BUILD_FOLDER = build-release
endif

.PHONY: build-libuv
build-libuv:
	ls -la lib/libuv
	ls -la lib/cpp-driver
	echo $(shell pwd)
	@mkdir -p $(BUILD_FOLDER)/libuv
	@cd $(BUILD_FOLDER)/libuv && \
	cmake -G "Ninja Multi-Config" \
		-DCMAKE_C_FLAGS="-fPIC" \
	    -DLIBUV_BUILD_TESTS=OFF \
	    -DLIBUV_BUILD_BENCH=OFF  ../../lib/libuv
	@cd $(BUILD_FOLDER)/libuv && ninja -f build-$(BUILD_TYPE).ninja uv_a
	@cd $(BUILD_FOLDER)/libuv && ninja -f build-$(BUILD_TYPE).ninja install

.PHONY: build-libcassandra
build-libcassandra: build-libuv
	@mkdir -p $(BUILD_FOLDER)/libcassandra
	@cd $(BUILD_FOLDER)/libcassandra && \
      cmake -G "Ninja Multi-Config" \
		-DCMAKE_CXX_FLAGS="-fPIC" \
		-DCMAKE_C_FLAGS="-fPIC" \
		-DCASS_USE_STATIC_LIBS=ON \
		-DCASS_BUILD_STATIC=ON \
		-DCASS_BUILD_SHARED=OFF \
		-DCASS_USE_TIMERFD=ON \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		../../lib/cpp-driver
	@cd $(BUILD_FOLDER)/libcassandra && ninja -f build-$(BUILD_TYPE).ninja cassandra_static
	@cd $(BUILD_FOLDER)/libcassandra && ninja -f build-$(BUILD_TYPE).ninja install

.PHONY: build
build: build-libuv build-libcassandra
	@export CMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@mkdir -p $(BUILD_FOLDER)/cassandra
	@cd $(BUILD_FOLDER)/cassandra \
	&& cmake \
		-G "Ninja Multi-Config" \
		-DCMAKE_CXX_FLAGS="-fPIC" \
		-DCMAKE_C_FLAGS="-fPIC" \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DLINK_STATICALLY=ON ../..
	@cd $(BUILD_FOLDER)/cassandra && ninja -f build-$(BUILD_TYPE).ninja ext-cassandra

install:
	cd $(BUILD_FOLDER)/cassandra && ninja -f build-$(BUILD_TYPE).ninja install

clean:
	@cd $(BUILD_FOLDER) && ninja clean

.PHONY: docker-dev-image
docker-dev-image:
	@docker build \
		--build-arg "IMAGE=malusevd99/php-ext-dev:8.1-debug" \
		-t "ghcr.io/nano-interactive/cassandra-php-driver:dev" \
		--target dev \
		--compress .

.PHONY: docker-production-image
docker-production-image:
	@docker build \
		--build-arg "IMAGE=malusevd99/php-ext-dev:8.1" \
		-t "ghcr.io/nano-interactive/cassandra-php-driver" \
		--target production \
		--compress .
.PHONY: php-test
php-test:
	@cd ext && php run-tests.php -j6 -m -c ../php.ini -d "error_reporting=E_ALL&~E_NOTICE&~E_DEPRECATED" tests/

.PHONY: php-arg-info
php-arg-info:
	@php $(shell pwd)/ext/build/gen_stub.php \
		-f \
		--generate-classsynopses \
		--generate-methodsynopses \
		--parameter-stats \
		ext/src/

.PHONY: run-dev-image
run-dev-image:
	@docker run \
		--rm \
		-it \
		-v "$(shell pwd):/tmp/cassandra-php-driver" \
		-w "/tmp/cassandra-php-driver" \
		ghcr.io/nano-interactive/cassandra-php-driver:dev \
		bash
.PHONY: run-prod-image
run-prod-image:
	@docker run \
		--rm \
		-it \
		-v "$(shell pwd):/tmp/cassandra-php-driver" \
		-w "/tmp/cassandra-php-driver" \
		ghcr.io/nano-interactive/cassandra-php-driver \
		bash