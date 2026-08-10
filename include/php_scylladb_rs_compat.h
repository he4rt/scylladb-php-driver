/**
 * Copyright 2026 ScyllaDB PHP driver authors.
 *
 * Licensed under the Apache License, Version 2.0.
 */
#pragma once

#include <cassandra.h>

/*
 * Schema-metadata shims for the scylla-rust backend.
 *
 * cpp-rs-driver up to v1.0.x still declared these seven functions, but never
 * implemented them meaningfully: they were documented under "Functions
 * intentionally not implemented" and returned nothing useful. Its master branch
 * removed the declarations, which turns every call site into a compile error.
 *
 * Behaviour on that backend does not change, because it was already empty. The
 * call sites all null-check the returned pointer or iterator, so returning
 * nullptr here reproduces exactly what the real functions did: schema
 * introspection of table, column, keyspace and materialized-view field data
 * yields empty results, and everything else keeps working.
 *
 * If a future cpp-rs-driver implements any of these for real, the duplicate
 * declaration is a loud compile error rather than a silent shadow, which is
 * the failure mode we want.
 */

#ifdef PHP_SCYLLADB_BACKEND_SCYLLA_RUST

static inline const CassValue *cass_keyspace_meta_field_by_name(const CassKeyspaceMeta *keyspace_meta,
                                                                const char *name)
{
    (void)keyspace_meta;
    (void)name;
    return nullptr;
}

static inline const CassValue *cass_table_meta_field_by_name(const CassTableMeta *table_meta,
                                                             const char *name)
{
    (void)table_meta;
    (void)name;
    return nullptr;
}

static inline const CassValue *cass_column_meta_field_by_name(const CassColumnMeta *column_meta,
                                                              const char *name)
{
    (void)column_meta;
    (void)name;
    return nullptr;
}

/* Returning an error keeps the caller's `== CASS_OK` guard false, so it skips
 * the field instead of reading an uninitialised name. */
static inline CassError cass_iterator_get_meta_field_name(const CassIterator *iterator, const char **name,
                                                          size_t *name_length)
{
    (void)iterator;
    (void)name;
    (void)name_length;
    return CASS_ERROR_LIB_NOT_IMPLEMENTED;
}

static inline const CassValue *cass_iterator_get_meta_field_value(const CassIterator *iterator)
{
    (void)iterator;
    return nullptr;
}

/* Callers null-check the iterator before use and skip the loop entirely. */
static inline CassIterator *cass_iterator_fields_from_table_meta(const CassTableMeta *table_meta)
{
    (void)table_meta;
    return nullptr;
}

static inline CassIterator *cass_iterator_fields_from_materialized_view_meta(
    const CassMaterializedViewMeta *view_meta)
{
    (void)view_meta;
    return nullptr;
}

#endif /* PHP_SCYLLADB_BACKEND_SCYLLA_RUST */
