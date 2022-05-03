LDFLAGS ?= -L/usr/local/lib
LIBS ?= -lssl -lz -luv -lm -lgmp -lstdc++

.PHONY: build
build:
	mkdir -p build
	cd build \
	&& cmake \
		-G "Ninja Multi-Config" \
		-DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -fPIC" \
		-DCMAKE_C_FLAGS="${CMAKE_C_FLAGS} -fPIC" \
		-DCASS_USE_STATIC_LIBS=ON \
		-DCASS_BUILD_STATIC=ON \
		-DCASS_BUILD_SHARED=OFF \
		-DCASS_USE_TIMERFD=ON \
		-DLIBUV_LIBRARY=./lib/libuv \
		-DCMAKE_BUILD_TYPE=RELEASE \
		-DLINK_STATICALLY=ON .. \
	&& ninja libuv_a.a \
	&& ninja cassandra_static \
	&& ninja ext-cassandra
install:
	cd build && ninja -f build.ninja install

clean:
	@cd build && ninja clean

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