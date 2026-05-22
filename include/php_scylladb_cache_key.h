/**
 * Copyright 2026 ScyllaDB PHP driver authors.
 *
 * Licensed under the Apache License, Version 2.0.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include <Zend/zend_long.h>
#include <Zend/zend_string.h>

/*
 * FNV-1a 64-bit fingerprint hash for EG(persistent_list) cache keys.
 *
 * Replaces the old `spprintf + zend_string_init` pattern that allocated
 * a fresh string on every connect()/prepare() call (even on cache hit).
 * The persistent_list is now keyed by uint64_t — no per-call allocation,
 * O(1) integer-key comparison, and 64-bit collision probability is
 * negligible for the entry counts a worker ever holds.
 *
 * Stability: as long as the field set mixed by a given call site doesn't
 * change between requests, the fingerprint is stable across requests
 * within the same worker — which is what the persistent_list cache
 * needs.
 */

#define PHP_SCYLLADB_FNV1A_OFFSET 0xcbf29ce484222325ULL
#define PHP_SCYLLADB_FNV1A_PRIME  0x100000001b3ULL

static zend_always_inline zend_ulong php_scylladb_cache_key_init(void) {
    return PHP_SCYLLADB_FNV1A_OFFSET;
}

static zend_always_inline zend_ulong php_scylladb_cache_key_mix_byte(zend_ulong h, uint8_t b) {
    return (h ^ b) * PHP_SCYLLADB_FNV1A_PRIME;
}

static zend_always_inline zend_ulong php_scylladb_cache_key_mix_bytes(zend_ulong h, const void *buf, size_t len) {
    const uint8_t *p = (const uint8_t *)buf;
    for (size_t i = 0; i < len; i++) {
        h = (h ^ p[i]) * PHP_SCYLLADB_FNV1A_PRIME;
    }
    return h;
}

static zend_always_inline zend_ulong php_scylladb_cache_key_mix_long(zend_ulong h, zend_long v) {
    return php_scylladb_cache_key_mix_bytes(h, &v, sizeof(v));
}

static zend_always_inline zend_ulong php_scylladb_cache_key_mix_int(zend_ulong h, int v) {
    return php_scylladb_cache_key_mix_bytes(h, &v, sizeof(v));
}

static zend_always_inline zend_ulong php_scylladb_cache_key_mix_ulong(zend_ulong h, zend_ulong v) {
    return php_scylladb_cache_key_mix_bytes(h, &v, sizeof(v));
}

/* Mix a zend_string, including a separator byte so adjacent fields can't
   alias (e.g. ("ab", "c") vs ("a", "bc")). NULL becomes a single 0 byte. */
static zend_always_inline zend_ulong php_scylladb_cache_key_mix_zstr(zend_ulong h, const zend_string *s) {
    if (s == NULL) {
        return php_scylladb_cache_key_mix_byte(h, 0);
    }
    h = php_scylladb_cache_key_mix_bytes(h, ZSTR_VAL(s), ZSTR_LEN(s));
    return php_scylladb_cache_key_mix_byte(h, 0);  /* separator */
}

/* Mix a C string (NUL-terminated or NULL). */
static zend_always_inline zend_ulong php_scylladb_cache_key_mix_cstr(zend_ulong h, const char *s) {
    if (s == NULL) {
        return php_scylladb_cache_key_mix_byte(h, 0);
    }
    while (*s) {
        h = (h ^ (uint8_t)*s++) * PHP_SCYLLADB_FNV1A_PRIME;
    }
    return php_scylladb_cache_key_mix_byte(h, 0);
}
