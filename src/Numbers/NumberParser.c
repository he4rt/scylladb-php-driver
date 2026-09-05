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

#include <ctype.h>
#include <errno.h>
#include <gmp.h>
#include <stdlib.h>

#include <php_scylladb.h>
#include <php_scylladb_types.h>

#include "Numbers/NumberParser.h"

static bool
prepare_string_conversion(const char* in, int in_len, int* pos, int* negative, int* base)
{
  int point = 0;

  *negative = 0;
  *base     = 0;
  *pos      = 0;

  if (in_len < 0 || (size_t) in_len != strlen(in)) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
                            "Value of %d bytes contains a NUL byte at offset %zu", in_len,
                            strlen(in));
    return false;
  }

  while (point < in_len && isspace((unsigned char) in[point])) {
    point++;
  }

  if (point < in_len && (in[point] == '+' || in[point] == '-')) {
    *negative = in[point] == '-';
    point++;
  }

  /* Handle special case for binary e.g. "0b0100" */
  if (point + 1 < in_len && in[point] == '0' && in[point + 1] == 'b') {
    *base = 2;
    point += 2; /* Skip over "0b" */
  }

  *pos = point;

  return point < in_len && isdigit((unsigned char) in[point]);
}

bool
php_scylladb_parse_float(const zend_string* zstr, cass_float_t* number )
{
  const char* in = ZSTR_VAL(zstr);
  const int in_len = (int)ZSTR_LEN(zstr);
  char* end;
  errno = 0;

  *number = (cass_float_t) strtof(in, &end);

  if (errno == ERANGE) {
    zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 , "Value is too small or too big for float: '%.*s'", in_len, in);
    return false;
  }

  if (errno || end == in) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Invalid float value: '%.*s'", in_len, in);
    return false;
  }

  if (end != &in[in_len]) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Invalid characters were found in value: '%.*s'", in_len, in);
    return false;
  }

  return true;
}

bool
php_scylladb_parse_double(const zend_string* zstr, cass_double_t* number )
{
  const char* in = ZSTR_VAL(zstr);
  const int in_len = (int)ZSTR_LEN(zstr);
  char* end;
  errno = 0;

  *number = (cass_double_t) strtod(in, &end);

  if (errno == ERANGE) {
    zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 , "Value is too small or too big for double: '%.*s'", in_len, in);
    return false;
  }

  if (errno || end == in) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Invalid double value: '%.*s'", in_len, in);
    return false;
  }

  if (end != &in[in_len]) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Invalid characters were found in value: '%.*s'", in_len, in);
    return false;
  }

  return true;
}

bool
php_scylladb_parse_int(const zend_string* zstr, cass_int32_t* number )
{
  const char* in = ZSTR_VAL(zstr);
  const int in_len = (int)ZSTR_LEN(zstr);
  char* end          = nullptr;
  int pos            = 0;
  int negative       = 0;
  cass_uint32_t temp = 0;
  int base           = 0;

  if (!prepare_string_conversion(in, in_len, &pos, &negative, &base)) {
    if (!EG(exception)) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
                              "Invalid integer value: '%.*s'", in_len, in);
    }
    return false;
  }

  errno = 0;
  temp  = (cass_uint32_t) strtoul(in + pos, &end, base);

  if (negative) {
    if (temp > (cass_uint32_t) INT_MAX + 1) {
      errno   = ERANGE;
      *number = INT_MIN;
    } else if (temp == (cass_uint32_t) INT_MAX + 1) {
      *number = INT_MIN;
    } else {
      *number = -((cass_int32_t) temp);
    }
  } else {
    if (temp > (cass_uint32_t) INT_MAX) {
      errno   = ERANGE;
      *number = INT_MAX;
    } else {
      *number = (cass_int32_t) temp;
    }
  }

  if (errno == ERANGE) {
    /* Signal the overflow through errno (still ERANGE here) and return without
     * throwing. The only callers are Tinyint/Smallint, which emit their own
     * narrower "must be between ..." range error; throwing here would both
     * report the wrong (32-bit) bounds and clobber errno via the exception
     * machinery before the caller can inspect it. */
    return false;
  }

  if (errno || end == &in[pos]) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Invalid integer value: '%.*s'", in_len, in);
    return false;
  }

  if (end != &in[in_len]) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Invalid characters were found in value: '%.*s'", in_len, in);
    return false;
  }

  return true;
}

bool
php_scylladb_parse_bigint(const zend_string* zstr, cass_int64_t* number )
{
  const char* in = ZSTR_VAL(zstr);
  const int in_len = (int)ZSTR_LEN(zstr);
  char* end          = nullptr;
  int pos            = 0;
  int negative       = 0;
  cass_uint64_t temp = 0;
  int base           = 0;

  if (!prepare_string_conversion(in, in_len, &pos, &negative, &base)) {
    if (!EG(exception)) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
                              "Invalid integer value: '%.*s'", in_len, in);
    }
    return false;
  }

  errno = 0;
  temp  = (cass_uint64_t) strtoull(in + pos, &end, base);

  if (negative) {
    if (temp > (cass_uint64_t) INT64_MAX + 1) {
      errno   = ERANGE;
      *number = INT64_MIN;
    } else if (temp == (cass_uint64_t) INT64_MAX + 1) {
      *number = INT64_MIN;
    } else {
      *number = -((cass_int64_t) temp);
    }
  } else {
    if (temp > (cass_uint64_t) INT64_MAX) {
      errno   = ERANGE;
      *number = INT64_MAX;
    } else {
      *number = (cass_int64_t) temp;
    }
  }

  if (errno == ERANGE) {
    zend_throw_exception_ex(php_scylladb_range_exception_ce, 0 ,
                            "value must be between %" PRId64 " and %" PRId64 ", %s given", INT64_MIN, INT64_MAX, in);
    return false;
  }

  if (errno || end == &in[pos]) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Invalid integer value: '%.*s'", in_len, in);
    return false;
  }

  if (end != &in[in_len]) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Invalid characters were found in value: '%.*s'", in_len, in);
    return false;
  }

  return true;
}

bool
php_scylladb_parse_varint(const zend_string* zstr, mpz_t* number )
{
  const char* in = ZSTR_VAL(zstr);
  const int in_len = (int)ZSTR_LEN(zstr);
  int pos      = 0;
  int negative = 0;
  int base     = 0;

  if (!prepare_string_conversion(in, in_len, &pos, &negative, &base) ||
      mpz_set_str(*number, &in[pos], base) == -1) {
    if (!EG(exception)) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0,
                              "Invalid integer value: '%.*s'", in_len, in);
    }
    return false;
  }

  if (negative)
    mpz_neg(*number, *number);

  return true;
}

bool
php_scylladb_parse_decimal(const zend_string* zstr, mpz_t* number, long* scale )
{
  const char* in = ZSTR_VAL(zstr);
  const int in_len = (int)ZSTR_LEN(zstr);
  /*  start is the index into the char array where the significand starts */
  int start = 0;
  /*
   *  point is the index into the char array where the exponent starts
   *  (or, if there is no exponent, this is equal to end)
   */
  int point = 0;
  /*
   * dot is the index into the char array where the decimal point is
   * found, or -1 if there is no decimal point
   */
  int dot = -1;
  /*
   * out will be storing the string representation of the integer part
   * of the decimal value. It is allocated only once the input is known to
   * be a real decimal — the hex/binary/octal paths below hand off to
   * php_scylladb_parse_varint() and never touch it.
   */
  char* out = nullptr;
  /*  holds length of the formatted integer number */
  int out_len = 0;

  int maybe_octal = 0;

  /*
   * The following examples show what these variables mean.  Note that
   * point and dot don't yet have the correct values, they will be
   * properly assigned in a loop later on in this method.
   *
   * Example 1
   *
   *        +  1  0  2  .  4  6  9
   * __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
   *
   * offset = 2, in_len = 8, start = 3, dot = 6, point = end = 10
   *
   * Example 2
   *
   *        +  2  3  4  .  6  1  3  E  -  1
   * __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
   *
   * offset = 2, in_len = 11, start = 3, dot = 6, point = 10, end = 13
   *
   * Example 3
   *
   *        -  1  2  3  4  5  e  7
   * __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
   *
   * offset = 2, in_len = 8, start = 3, dot = -1, point = 8, end = 10
   */

  /* Determine the sign of the number. */
  int negative = 0;
  if (in[start] == '+') {
    start++;
    point++;
  } else if (in[start] == '-') {
    start++;
    point++;
    negative = 1;
  }

  maybe_octal = (in[point] == '0');

  /* Hex or binary */
  if (maybe_octal && (in[point + 1] == 'b' || in[point + 1] == 'x')) {
    *scale = 0;
    return php_scylladb_parse_varint(zstr, number );
  }

  /*
   * Check each character looking for the decimal point and the
   * start of the exponent.
   */
  while (point < in_len) {
    char c = in[point];

    if (c == '.') {
      /* If dot != -1 then we've seen more than one decimal point. */
      if (dot != -1) {
        zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Multiple '.' (dots) in the number '%.*s'", in_len, in);
        return false;
      }

      dot = point;
    }
    /* Break when we reach the start of the exponent. */
    else if (c == 'e' || c == 'E')
      break;
    /*
     * Throw an exception if the character was not a decimal or an
     * exponent and is not a hexadecimal digit.
     */
    else if (!isxdigit(c)) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Unrecognized character '%c' at position %d", c, point);
      return false;
    }

    point++;
  }

  /* Octal number */
  if (maybe_octal && dot == -1) {
    *scale = 0;
    return php_scylladb_parse_varint(zstr, number );
  }

  out = ecalloc((size_t)(in_len + 1), sizeof(char));

  /* Prepend a negative sign if necessary. */
  if (negative)
    out[0] = '-';

  if (dot != -1) {
    /*
     * If there was a decimal we must combine the two parts that
     * contain only digits and we must set the scale properly.
     */
    memcpy(&out[negative], &in[start], (size_t)(dot - start));
    memcpy(&out[negative + dot - start], &in[dot + 1], (size_t)(point - dot - 1));

    out_len = point - start + negative - 1;
    *scale  = point - 1 - dot;
  } else {
    /*
     * If there was no decimal then the unscaled value is just the number
     * formed from all the digits and the scale is zero.
     */
    memcpy(&out[negative], &in[start], (size_t)(point - start));
    out_len = point - start + negative;
    *scale  = 0;
  }

  if (out_len == 0) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "No digits seen in value: '%.*s'", in_len, in);
    efree(out);
    return false;
  }

  if (mpz_set_str(*number, out, 10) == -1) {
    zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Unable to extract integer part of decimal value: '%.*s', %s",
                            in_len, in, out);
    efree(out);
    return false;
  }

  efree(out);

  /*
   * Now parse exponent.
   * If point < end that means we broke out of the previous loop when we
   * saw an 'e' or an 'E'.
   */
  if (point < in_len) {
    char* exponent_end;
    long diff;

    point++;
    /* Ignore a '+' sign. */
    if (in[point] == '+')
      point++;

    /*
     * Throw an exception if there were no digits found after the 'e'
     * or 'E'.
     */
    if (point >= in_len) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "No exponent following e or E in value: '%.*s'", in_len, in);
      return false;
    }

    /* strtol() skips leading whitespace and stops at the first character it
       cannot use. Reject anything it would silently accept or leave behind, so
       "1e 2" and "1e2foo" fail instead of parsing as 1e2. */
    if (in[point] != '-' && !isdigit((unsigned char) in[point])) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Malformed exponent in value: '%.*s'", in_len, in);
      return false;
    }

    errno = 0;
    diff = strtol(&in[point], &exponent_end, 10);
    if (exponent_end != &in[in_len] || errno == ERANGE) {
      zend_throw_exception_ex(php_scylladb_invalid_argument_exception_ce, 0 , "Malformed exponent in value: '%.*s'", in_len, in);
      return false;
    }

    *scale = *scale - diff;
  }

  return true;
}

zend_string*
php_scylladb_format_integer(mpz_t number)
{
  /* Adding 2 ensures enough space for the null-terminator and negative sign */
  zend_string* out = zend_string_alloc(mpz_sizeinbase(number, 10) + 2, 0);
  mpz_get_str(ZSTR_VAL(out), 10, number);

  return zend_string_truncate(out, strlen(ZSTR_VAL(out)), 0);
}

zend_string*
php_scylladb_format_decimal(mpz_t number, long scale)
{
  char* tmp    = nullptr;
  size_t total = 0;
  size_t len   = 0;
  size_t need  = 0;
  int negative = 0;
  long point   = 0;
  bool plain   = false;

  if (scale == 0) {
    return php_scylladb_format_integer(number);
  }

  if (mpz_sgn(number) < 0)
    negative = 1;

  // GMP needs mpz_sizeinbase() + 2 bytes for the digits, the sign and the terminator.
  // mpz_sizeinbase() can overshoot by one, so the exact length is only known after
  // the conversion; the buffer is grown to the final size below.
  tmp = emalloc(mpz_sizeinbase(number, 10) + 2);
  mpz_get_str(tmp, 10, number);

  // Update len to be the true length of the string representation of |number|.
  // NOTE: the length of the string includes the negative sign (if present); account for that.
  len = strlen(tmp) - (size_t)negative;

  point = (long) len - scale;

  // Plain notation applies to a non-negative scale with an adjusted exponent of at least
  // -6, which is the rule BigDecimal.toString() uses. A negative scale always gets
  // scientific notation, so an unscaled 1 with scale -3 prints as 1E+3, not as 1000.
  plain = scale > 0 && (point - 1) >= -6;

  if (plain) {
    need = point <= 0 ? (size_t) negative + 2 + (size_t) -point + len + 1 : len + (size_t) negative + 2;
  } else {
    // digits, sign, decimal point, then "E" and a signed long exponent.
    need = len + (size_t) negative + 24;
  }
  tmp = erealloc(tmp, need);

  if (plain) {
    if (point <= 0) {
      // e.g. -0.002 and 0.002
      int shift_start = negative;

      // current position
      int i = 0;

      // Move the numeric part (skip leading minus if needed) of tmp right by enough bytes to make room for
      // 0.0000 (as many leading zeroes as necessary).
      memmove(&(tmp[shift_start + 2 - point]), &(tmp[shift_start]), len);

      // This is a (possibly negative) number with a 0 integer part.
      if (negative)
        tmp[i++] = '-';

      tmp[i++] = '0';
      tmp[i++] = '.';

      // Add leading zeroes.
      while (point < 0) {
        tmp[i++] = '0';
        point++;
      }

      total      = (size_t)i + len;
      tmp[total] = '\0';
    } else {
      // e.g. 1.2, -1.2
      /* absolute length + negative sign + point sign */
      total = len + (size_t)negative + 1;

      // Insert the decimal point at the right location in the string.

      // point is the index at which to insert the decimal point, but it assumes we have a positive
      // number. Move it to the right if we have a negative number.
      point += negative;

      memmove(&(tmp[point + 1]), &(tmp[point]), total - (size_t)point);

      tmp[point] = '.';
      tmp[total] = '\0';
    }
  } else {
    // A number we want to express in scientific notation: either very small
    // (0.000000004, -0.000000004) or held with a negative scale (1E+3).

    // The adjusted exponent, i.e. the exponent that goes with a single leading digit.
    long exponent = point - 1;
    int written   = 0;

    // If we only have one significant digit, we want to produce a string like
    // 1E-9. If we have more significant digits, then 1.123E-9.

    if (len == 1) {
      // Simple case; tmp is already leading with our number as we want it. Append E(exp) to it
      // and we're done.
      const size_t at = (size_t) 1 + (size_t) negative;
      written         = snprintf(&(tmp[at]), need - at, "E%+ld", exponent);
      total           = at + (size_t) written;
    } else {
      // We have a more complex number. Insert a decimal point after the first digit.
      int dot = negative ? 2 : 1;
      memmove(&(tmp[dot + 1]), &(tmp[dot]), len - 1);
      tmp[dot] = '.';

      // Now append the exponent to the end and we're done.
      const size_t at = (size_t) dot + len;
      written         = snprintf(&(tmp[at]), need - at, "E%+ld", exponent);
      total           = at + (size_t) written;
    }
  }

  zend_string* out = zend_string_init(tmp, total, 0);
  efree(tmp);

  return out;
}

void
import_twos_complement(const cass_byte_t* data, size_t size, mpz_t* number)
{
  mpz_import(*number, size, 1, sizeof(cass_byte_t), 1, 0, data);

  /* negative value */
  if ((data[0] & 0x80) == 0x80) {
    /* mpz_import() imports the two's complement value as an unsigned integer
     * so this needs to subtract 2^(8 * num_bytes) to get the negative value.
     */
    mpz_t temp;
    mpz_init(temp);
    mpz_set_ui(temp, 1);
    mpz_mul_2exp(temp, temp, 8 * size);
    mpz_sub(*number, *number, temp);
    mpz_clear(temp);
  }
}

cass_byte_t*
export_twos_complement(mpz_t number, size_t* size)
{
  cass_byte_t* bytes;

  if (mpz_sgn(number) == 0) {
    /* mpz_export() returns nullptr for 0 */
    bytes  = (cass_byte_t*) pemalloc(sizeof(cass_byte_t), 1);
    *bytes = 0;
    *size  = 1;
  } else if (mpz_sgn(number) == -1) {
    /*  mpz_export() ignores sign and only exports abs(number)
     *  so this needs to convert the number to the two's complement
     *  unsigned value.
     */
    size_t n;
    mpz_t temp;

    /* determine the number of bytes used in the two's complement
     * respresentation.
     */
    n = mpz_sizeinbase(number, 2) / 8 + 1;

    /* there's a special case for -2^(8 * n) numbers e.g. -128 (1000 0000) and
     * -32768 (100 0000 0000 0000), etc. that can be handled by n - 1 bytes in
     *  two's complement.
     */
    if (mpz_scan1(number, 0) == (8 * (n - 1)) - 1) {
      n--;
    }

    /* Add 2^(8 * num_bytes) to get the unsigned value e.g.
     * -1   + 2^8 = 255
     * -128 + 2^8 = 128
     * -129 + 2^16 = 65407
     * -32768 + 2^16 = 32768
     *  ...
     */
    mpz_init(temp);
    mpz_set_ui(temp, 1);
    mpz_mul_2exp(temp, temp, 8 * n);
    mpz_add(temp, number, temp);
    bytes = (cass_byte_t*) mpz_export(nullptr, size, 1, sizeof(cass_byte_t), 1, 0, temp);
    mpz_clear(temp);
  } else {
    /* mpz_export() always returns a unsigned number and can have
     * values where the most significate bit is set. A 0 byte prevents
     * these from being interpreted as a negative value in two's complement
     */

    /* round to the nearest byte and add space for a leading 0 byte */
    *size    = (mpz_sizeinbase(number, 2) + 7) / 8 + 1;
    bytes    = (cass_byte_t*) pemalloc(*size, 1);
    bytes[0] = 0;
    mpz_export(bytes + 1, nullptr, 1, sizeof(cass_byte_t), 1, 0, number);
  }

  return bytes;
}
