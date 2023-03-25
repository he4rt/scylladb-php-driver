#include <cstdint>

#if defined(ZENDCPP_SUPPORTS_SSE41) || defined(ZENDCPP_SUPPORTS_AVX2) ||       \
    defined(ZENDCPP_SUPPORTS_AVX512)
#include <immintrin.h>
#endif

#include "../ZendCPP.hpp"
#include "String.h"

namespace ZendCPP {
#ifdef ZENDCPP_SUPPORTS_SSE41
static const __m128i ascii_table =
    _mm_setr_epi8('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b',
                  'c', 'd', 'e', 'f');

static ZENDCPP_ALWAYS_INLINE void hex_encode_sse41(const uint8_t *input,
                                                   size_t length,
                                                   uint8_t *output) noexcept {
  for (size_t i = 0; i < length; i += 16) {
    __m128i data =
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(input + i));
    __m128i high_nibbles = _mm_srli_epi16(data, 4);

    __m128i low_nibbles = _mm_and_si128(data, _mm_set1_epi8(0x0F));
    __m128i interleaved_nibbles = _mm_unpacklo_epi8(high_nibbles, low_nibbles);

    // Use shuffle_epi8 to look up the ASCII values of the high and low nibbles
    __m128i high_ascii = _mm_shuffle_epi8(ascii_table, interleaved_nibbles);
    __m128i low_ascii =
        _mm_shuffle_epi8(ascii_table, _mm_srli_epi64(interleaved_nibbles, 4));

    // Store the high and low ASCII values in the output buffer
    _mm_storeu_si128(reinterpret_cast<__m128i *>(output + 2 * i), high_ascii);
    _mm_storeu_si128(reinterpret_cast<__m128i *>(output + 2 * i + 16),
                     low_ascii);
  }
}
#endif

#ifdef ZENDCPP_SUPPORTS_AVX2

const __m256i ascii_table_avx2 =
    _mm256_setr_epi8('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b',
                     'c', 'd', 'e', 'f', '0', '1', '2', '3', '4', '5', '6', '7',
                     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f');

void ZENDCPP_ALWAYS_INLINE hex_encode_avx2_double(const uint8_t *input,
                                                  size_t length,
                                                  uint8_t *output) {
  for (size_t i = 0; i < length; i += 64) {
    __m256i data1 =
        _mm256_loadu_si256(reinterpret_cast<const __m256i *>(input + i));
    __m256i data2 =
        _mm256_loadu_si256(reinterpret_cast<const __m256i *>(input + i + 32));

    __m256i high_nibbles1 = _mm256_srli_epi16(data1, 4);
    __m256i high_nibbles2 = _mm256_srli_epi16(data2, 4);

    __m256i low_nibbles1 = _mm256_and_si256(data1, _mm256_set1_epi8(0x0F));
    __m256i low_nibbles2 = _mm256_and_si256(data2, _mm256_set1_epi8(0x0F));

    __m256i interleaved_nibbles1 =
        _mm256_unpacklo_epi8(high_nibbles1, low_nibbles1);
    __m256i interleaved_nibbles2 =
        _mm256_unpacklo_epi8(high_nibbles2, low_nibbles2);

    __m256i high_ascii1 =
        _mm256_shuffle_epi8(ascii_table_avx2, interleaved_nibbles1);
    __m256i low_ascii1 = _mm256_shuffle_epi8(
        ascii_table_avx2, _mm256_srli_epi64(interleaved_nibbles1, 4));

    __m256i high_ascii2 =
        _mm256_shuffle_epi8(ascii_table_avx2, interleaved_nibbles2);
    __m256i low_ascii2 = _mm256_shuffle_epi8(
        ascii_table_avx2, _mm256_srli_epi64(interleaved_nibbles2, 4));

    _mm256_storeu_si256(reinterpret_cast<__m256i *>(output + 2 * i),
                        high_ascii1);
    _mm256_storeu_si256(reinterpret_cast<__m256i *>(output + 2 * i + 32),
                        low_ascii1);
    _mm256_storeu_si256(reinterpret_cast<__m256i *>(output + 2 * i + 64),
                        high_ascii2);
    _mm256_storeu_si256(reinterpret_cast<__m256i *>(output + 2 * i + 96),
                        low_ascii2);
  }
}

void ZENDCPP_ALWAYS_INLINE hex_encode_avx2(const uint8_t *input, size_t length,
                                           uint8_t *output) {
  for (size_t i = 0; i < length; i += 32) {
    __m256i data =
        _mm256_loadu_si256(reinterpret_cast<const __m256i *>(input + i));

    __m256i high_nibbles = _mm256_srli_epi16(data, 4);

    __m256i low_nibbles = _mm256_and_si256(data, _mm256_set1_epi8(0x0F));

    __m256i interleaved_nibbles =
        _mm256_unpacklo_epi8(high_nibbles, low_nibbles);

    __m256i high_ascii =
        _mm256_shuffle_epi8(ascii_table_avx2, interleaved_nibbles);
    __m256i low_ascii = _mm256_shuffle_epi8(
        ascii_table_avx2, _mm256_srli_epi64(interleaved_nibbles, 4));

    _mm256_storeu_si256(reinterpret_cast<__m256i *>(output + 2 * i),
                        high_ascii);
    _mm256_storeu_si256(reinterpret_cast<__m256i *>(output + 2 * i + 32),
                        low_ascii);
  }
}

#endif

#ifdef ZENDCPP_SUPPORTS_AVX512
void ZENDCPP_ALWAYS_INLINE hex_encode_avx512(const uint8_t *input, size_t len,
                                             uint8_t *output) {
}
#endif

void ZENDCPP_ALWAYS_INLINE hex_encode(const uint8_t *src, size_t src_len,
                                      uint8_t *dst) noexcept {
#if defined(ZENDCPP_SUPPORTS_AVX512) || defined(ZENDCPP_SUPPORTS_AVX2)
  if (src_len >= 64) {
#if defined(ZENDCPP_SUPPORTS_AVX512)
    hex_encode_avx512(src, src_len, dst);
#else
    hex_encode_avx2_double(src, src_len, dst);
#endif
  }
#endif

#ifdef ZENDCPP_SUPPORTS_AVX2
  if (src_len >= 32) {
    hex_encode_avx2(src, src_len, dst);
  }
#endif

#ifdef ZENDCPP_SUPPORTS_SSE41
  if (src_len >= 16) {
    hex_encode_sse41(src, src_len, dst);
  }
#endif
}

} // namespace ZendCPP