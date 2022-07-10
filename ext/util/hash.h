#pragma once

#include "php_driver.h"

#define uthash_malloc(sz) emalloc(sz)
#define uthash_free(ptr,sz) efree(ptr)

#define HASH_FUNCTION(key, keylen, num_bkts, hashv, bkt) \
  hashv = php_driver_value_hash((zval*) key);            \
  bkt   = (hashv) & (num_bkts - 1U)
#define HASH_KEYCOMPARE(a, b, len) \
  php_driver_value_compare((zval*)a, (zval*)b )

#undef HASH_ADD /* Previously defined in Zend/zend_hash.h */

#include "util/uthash.h"

#define HASH_FIND_ZVAL(head, zvptr, out) \
    HASH_FIND(hh, head, zvptr, 0, out)

#define HASH_ADD_ZVAL(head, fieldname, add) \
   HASH_ADD_KEYPTR(hh, head, PHP5TO7_ZVAL_MAYBE_P(((add)->fieldname)), 0, add)

struct php_driver_map_entry_ {
  zval key;
  zval value;
  UT_hash_handle hh;
};

struct php_driver_set_entry_ {
  zval value;
  UT_hash_handle hh;
};

#define PHP_DRIVER_COMPARE(a, b) ((a) < (b) ? -1 : (a) > (b))

unsigned php_driver_value_hash(zval* zvalue );
int php_driver_value_compare(zval* zvalue1, zval* zvalue2 );
int php_driver_data_compare(const void* a, const void* b );

unsigned php_driver_mpz_hash(unsigned seed, mpz_t n);

static inline unsigned php_driver_bigint_hash(cass_int64_t value) {
  return (unsigned)(value ^ (value >> 32));
}

static inline unsigned php_driver_combine_hash(unsigned seed, unsigned  hashv) {
  return seed ^ (hashv + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}
