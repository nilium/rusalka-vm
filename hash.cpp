/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include "hash.h"


uint32_t hash32(char const *str, size_t const length, uint32_t const seed)
{
  static uint32_t const mask_left[16] = {
    0x00000000U, 0x80000000U, 0xC0000000U, 0xE0000000U,
    0xF0000000U, 0xF8000000U, 0xFC000000U, 0xFE000000U,
    0xFF000000U, 0xFF800000U, 0xFFC00000U, 0xFFE00000U,
    0xFFF00000U, 0xFFF80000U, 0xFFFC0000U, 0xFFFE0000U,
  };
  uint32_t hash = seed;
  constexpr uint32_t hbits = sizeof(hash) * 8;
  size_t index = 0;
  for (; index < length; ++index) {
    uint32_t const curchar = str[index];
    hash = hash * 439 + curchar * 23 + (static_cast<uint32_t>(index) + 257);
    uint32_t const shift =
      ((curchar & 0x9) | ((curchar & 0x10) >> 2) | ((curchar & 0x40) >> 5)) ^
      ((curchar & 0xA) >> 5) | ((curchar & 0x2) << 2) | ((curchar & 0x4) >> 1);
    hash = (hash << shift) | (hash & mask_left[shift]) >> (hbits - shift);
  }
  return hash;
}



uint64_t hash64(char const *str, size_t const length, uint64_t const seed)
{
  static uint64_t const mask_left[16] = {
    0x0000ULL << 48, 0x8000ULL << 48, 0xC000ULL << 48, 0xE000ULL << 48,
    0xF000ULL << 48, 0xF800ULL << 48, 0xFC00ULL << 48, 0xFE00ULL << 48,
    0xFF00ULL << 48, 0xFF80ULL << 48, 0xFFC0ULL << 48, 0xFFE0ULL << 48,
    0xFFF0ULL << 48, 0xFFF8ULL << 48, 0xFFFCULL << 48, 0xFFFEULL << 48,
  };
  uint64_t hash = seed;
  constexpr uint64_t hbits = sizeof(hash) * 8;
  size_t index = 0;
  for (; index < length; ++index) {
    uint64_t const curchar = str[index];
    hash = hash * 5741U + curchar * 23U + (static_cast<uint64_t>(index) + 257U);
    uint64_t const shift =
      ((curchar & 0x9) | ((curchar & 0x10) >> 2) | ((curchar & 0x40) >> 5)) ^
      ((curchar & 0xA) >> 5) | ((curchar & 0x2) << 2) | ((curchar & 0x4) >> 1);
    hash = (hash << shift) | (hash & mask_left[shift]) >> (hbits - shift);
  }
  return hash;
}
