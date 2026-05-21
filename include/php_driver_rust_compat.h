#pragma once
/*
 * Compatibility shims for scylladb/cpp-rs-driver.
 *
 * The Rust-backed driver removed schema-introspection helpers
 * (`*_meta_field_by_name`, `iterator_fields_from_*`, `iterator_get_meta_field_*`)
 * because the underlying ScyllaDB Rust driver does not yet expose these APIs.
 *
 * Rather than ripping out the legacy `src/Database/*` module right now, we
 * provide null-returning inline stubs so the extension still links. Schema
 * introspection calls that go through these paths return NULL on this backend,
 * which the existing code already handles (it checks for NULL before reading).
 *
 * Remove this file once `src/Database` is either ported off these APIs or
 * dropped entirely. Active only when PHP_DRIVER_BACKEND=scylla-rust.
 */

#ifdef PHP_DRIVER_BACKEND_SCYLLA_RUST

#include <cassandra.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline const CassValue*
cass_column_meta_field_by_name(const CassColumnMeta* meta, const char* name)
{
    (void)meta; (void)name;
    return NULL;
}

static inline const CassValue*
cass_keyspace_meta_field_by_name(const CassKeyspaceMeta* meta, const char* name)
{
    (void)meta; (void)name;
    return NULL;
}

static inline CassIterator*
cass_iterator_fields_from_table_meta(const CassTableMeta* meta)
{
    (void)meta;
    return NULL;
}

static inline CassIterator*
cass_iterator_fields_from_materialized_view_meta(const CassMaterializedViewMeta* meta)
{
    (void)meta;
    return NULL;
}

static inline CassError
cass_iterator_get_meta_field_name(const CassIterator* iterator,
                                  const char** name, size_t* name_length)
{
    (void)iterator;
    if (name) *name = NULL;
    if (name_length) *name_length = 0;
    return CASS_ERROR_LIB_NOT_IMPLEMENTED;
}

static inline const CassValue*
cass_iterator_get_meta_field_value(const CassIterator* iterator)
{
    (void)iterator;
    return NULL;
}

#ifdef __cplusplus
}
#endif

#endif /* PHP_DRIVER_BACKEND_SCYLLA_RUST */
