/**
 * Copyright 2015-2017 DataStax, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <cassandra.h>
#include <gmp.h>
#include <php.h>

void import_twos_complement(const cass_byte_t* data, size_t size, mpz_t* number);
cass_byte_t* export_twos_complement(mpz_t number, size_t* size);

[[nodiscard]] bool php_scylladb_parse_float(const zend_string* in, cass_float_t* number);
[[nodiscard]] bool php_scylladb_parse_double(const zend_string* in, cass_double_t* number);
[[nodiscard]] bool php_scylladb_parse_int(const zend_string* in, cass_int32_t* number);
[[nodiscard]] bool php_scylladb_parse_bigint(const zend_string* in, cass_int64_t* number);
[[nodiscard]] bool php_scylladb_parse_varint(const zend_string* in, mpz_t* number);
[[nodiscard]] bool php_scylladb_parse_decimal(const zend_string* in, mpz_t* number, long* scale);

[[nodiscard]] zend_string* php_scylladb_format_integer(mpz_t number);
[[nodiscard]] zend_string* php_scylladb_format_decimal(mpz_t number, long scale);
