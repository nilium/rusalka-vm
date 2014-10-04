/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <cstddef>
#include <cstdint>


/**
  Default 32-bit seed for hash32. It's more or less random garbage that
  produces better results than some other random garbage.
*/
const uint32_t DEFAULT_HASH_SEED_32 = 0x9E2030F1U;

/**
  Default 64-bit seed for hash64.
*/
const uint64_t DEFAULT_HASH_SEED_64 = 0x9E2030F19E2030F1ULL;



/**
  Produces a 32-bit hash of the input data.
  @param str    The input data.
  @param length The length of the input data.
  @param seed   The seed for the input. If you want to concatenate string
  hashes, you can use a previous string hash as the seed and it will be the
  same as hashing the combined strings.
*/
uint32_t hash32(char const *str, size_t const length,
                uint32_t const seed = DEFAULT_HASH_SEED_32);

/**
  Produces a 64-bit hash of the input data.
  @param str    The input data.
  @param length The length of the input data.
  @param seed   The seed for the input. If you want to concatenate string
  hashes, you can use a previous string hash as the seed and it will be the
  same as hashing the combined strings.
*/
uint64_t hash64(char const *str, size_t const length,
                uint64_t const seed = DEFAULT_HASH_SEED_64);
