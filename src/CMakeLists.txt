php_scylladb_module(src ext_scylladb::src)

target_sources(src PRIVATE
        Async/Reactor.c
        BatchStatement.c
        Blob.c
        Collection.c
        Core.c
        Custom.c
        ExecutionOptions.c
        FuturePreparedStatement.c
        FutureRows.c
        FutureSession.c
        FutureClose.c
        FutureNotifier.c
        FutureUtil.c
        FutureValue.c
        Inet.c
        InetUtil.c
        Map.c
        UuidGen.c
        DefaultSession.c
        PreparedStatement.c
        ProtocolVersion.c
        Set.c
        SimpleStatement.c
        Tuple.c
        Type.c
        UserTypeValue.c
        Uuid.c
)


# ── Native (Open)Swoole coroutine bridge (opt-in) ─────────────────────────────
# The ONLY C++ translation unit; compiled solely when a swoole flag is set.
# Swoole's own C++ symbols resolve lazily from the loaded (open)swoole extension
# at runtime, so we add its headers to the include path but do not link libswoole.
if (PHP_SCYLLADB_ENABLE_SWOOLE AND PHP_SCYLLADB_ENABLE_OPENSWOOLE)
    message(FATAL_ERROR
            "PHP_SCYLLADB_ENABLE_SWOOLE and PHP_SCYLLADB_ENABLE_OPENSWOOLE are "
            "mutually exclusive — a build targets one coroutine runtime. Enable "
            "only one and point PHP_SCYLLADB_SWOOLE_SRC at that source tree.")
endif ()

if (PHP_SCYLLADB_ENABLE_SWOOLE OR PHP_SCYLLADB_ENABLE_OPENSWOOLE)
    if (NOT PHP_SCYLLADB_SWOOLE_SRC OR NOT EXISTS "${PHP_SCYLLADB_SWOOLE_SRC}")
        message(FATAL_ERROR
                "Native Swoole support requires the source headers. Pass "
                "-DPHP_SCYLLADB_SWOOLE_SRC=/path/to/(open)swoole-src (the tree "
                "containing include/swoole_coroutine_system.h).")
    endif ()

    # The project declares LANGUAGES C; CXX is otherwise enabled late (by the
    # legacy .cpp modules added after src/). This subdir is configured before
    # them, so enable CXX explicitly here or CMake silently drops the .cc on a
    # fresh configure (it compiled locally only because CXX was cached).
    enable_language(CXX)

    target_sources(src PRIVATE Async/SwooleBridge.cc)
    # The bridge is C++; the rest of the target stays C23.
    set_target_properties(src PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
    target_include_directories(src PRIVATE
            "${PHP_SCYLLADB_SWOOLE_SRC}"
            "${PHP_SCYLLADB_SWOOLE_SRC}/include"
            "${PHP_SCYLLADB_SWOOLE_SRC}/ext-src")
    target_compile_definitions(src PRIVATE HAVE_SWOOLE_COROUTINE)
    if (PHP_SCYLLADB_ENABLE_OPENSWOOLE)
        target_compile_definitions(src PRIVATE PHP_SCYLLADB_OPENSWOOLE)
    endif ()
    message(STATUS "Native Swoole coroutine support: ON (src ${PHP_SCYLLADB_SWOOLE_SRC})")
endif ()

# ── Io\Poll handle (PHP >= 8.6) ───────────────────────────────────────────────
# Detected in the root CMakeLists.txt. The class, its stub and the whole
# translation unit are absent from a build against an older PHP.
if (PHP_SCYLLADB_HAVE_POLL_API)
    target_sources(src PRIVATE Async/PollHandle.c Async/Poll.c)
    target_compile_definitions(src PRIVATE HAVE_PHP_POLL_API)
    php_scylladb_generate_arginfo(src Async/PollHandle.stub.php Async/Poll.stub.php)
    # PollHandle registers itself by hand (its layout is php-src's
    # php_poll_handle_object, so the generator's zendObject offset does not fit).
    php_scylladb_generate_descriptor(src Async/Poll.stub.php)
    message(STATUS "Io\\Poll integration: ON (Cassandra\\Async\\Poll, PollHandle)")
endif ()

php_scylladb_generate_arginfo(src
        Async/Reactor.stub.php
        BatchStatement.stub.php
        Inet.stub.php
        Blob.stub.php
        Map.stub.php
        Cassandra.stub.php
        ProtocolVersion.stub.php
        PreparedStatement.stub.php
        Collection.stub.php
        Session.stub.php
        DefaultSession.stub.php
        Set.stub.php
        ExecutionOptions.stub.php
        SimpleStatement.stub.php
        Future.stub.php
        Statement.stub.php
        FutureClose.stub.php
        Tuple.stub.php
        FuturePreparedStatement.stub.php
        UserTypeValue.stub.php
        FutureRows.stub.php
        Uuid.stub.php
        FutureSession.stub.php
        UuidInterface.stub.php
        FutureValue.stub.php
        Value.stub.php
        # Type.cpp at this level #includes "Type/Type_arginfo.h" — needs to
        # be a generated-file dep of THIS target, not just the `type` target.
        Type/Type.stub.php
)

php_scylladb_generate_descriptor(src
        Async/Reactor.stub.php
        Type/Type.stub.php
        Value.stub.php
        UuidInterface.stub.php
        Blob.stub.php
        Inet.stub.php
        Uuid.stub.php
        Set.stub.php
        Map.stub.php
        Collection.stub.php
        Tuple.stub.php
        UserTypeValue.stub.php
        Cassandra.stub.php
        ProtocolVersion.stub.php
        Future.stub.php
        FuturePreparedStatement.stub.php
        FutureRows.stub.php
        FutureSession.stub.php
        FutureValue.stub.php
        FutureClose.stub.php
        Session.stub.php
        DefaultSession.stub.php
        Statement.stub.php
        SimpleStatement.stub.php
        PreparedStatement.stub.php
        BatchStatement.stub.php
        ExecutionOptions.stub.php
)
