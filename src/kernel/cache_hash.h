/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_CACHE_HASH_H)
#define      VITA_CACHE_HASH_H

#include "kernel/vita.h"

namespace vita
{
///
/// \param[in] data data stream to be hashed.
/// \param[in] len length, in bytes, of `data`.
/// \param[in] seed initialization seed.
/// \return the signature of `data`.
///
/// MurmurHash3 (https://github.com/aappleby/smhasher), by Austin Appleby, is a
/// relatively simple non-cryptographic hash algorithm. It's noted for being
/// fast, with excellent distribution, avalanche behavior and overall
/// collision resistance.
///
/// \note
/// MurmurHash and CityHash are excellent hash functions and are equally
/// portable.  In favor of MurmurHash: it's been around for longer and is
/// already used in many STL implementations.  In favor of CityHash: it
/// performs a bit better than Murmurhash, on average (at least on x86-64
/// architectures).
///
/// \see
/// * <http://comments.gmane.org/gmane.comp.compilers.clang.devel/18702>
/// * <http://blog.reverberate.org/2012/01/state-of-hash-functions-2012.html>
/// * <http://code.google.com/p/cityhash/>
/// * <http://code.google.com/p/smhasher/>
///
inline hash_t hash(void *const data, unsigned len, const unsigned seed)
{
  /// Murmurhash3 follows.
  const unsigned n_blocks(len / 16);  // Block size is 128bit

  std::uint64_t h1(seed), h2(seed);

  const std::uint64_t c1(0x87c37b91114253d5), c2(0x4cf5ad432745937f);

  // Body.
  const std::uint64_t *const blocks(
    reinterpret_cast<const std::uint64_t *>(data));

  for (unsigned i(0); i < n_blocks; ++i)
  {
    std::uint64_t k1(blocks[i * 2 + 0]);
    std::uint64_t k2(blocks[i * 2 + 1]);

    k1 *= c1;
    k1  = ROTL64(k1, 31);
    k1 *= c2;
    h1 ^= k1;

    h1 = ROTL64(h1, 27);
    h1 += h2;
    h1 = h1*5 + 0x52dce729;

    k2 *= c2;
    k2  = ROTL64(k2, 33);
    k2 *= c1;
    h2 ^= k2;

    h2 = ROTL64(h2, 31);
    h2 += h1;
    h2 = h2*5 + 0x38495ab5;
  }

  // Tail.
  const std::uint8_t *const tail(
    reinterpret_cast<const std::uint8_t *>(data) + n_blocks * 16);

  std::uint64_t k1(0), k2(0);

  switch (len & 15)
  {
  case 15: k2 ^= std::uint64_t(tail[14]) << 48;
  case 14: k2 ^= std::uint64_t(tail[13]) << 40;
  case 13: k2 ^= std::uint64_t(tail[12]) << 32;
  case 12: k2 ^= std::uint64_t(tail[11]) << 24;
  case 11: k2 ^= std::uint64_t(tail[10]) << 16;
  case 10: k2 ^= std::uint64_t(tail[ 9]) << 8;
  case  9: k2 ^= std::uint64_t(tail[ 8]) << 0;
           k2 *= c2; k2  = ROTL64(k2, 33); k2 *= c1; h2 ^= k2;

  case  8: k1 ^= std::uint64_t(tail[ 7]) << 56;
  case  7: k1 ^= std::uint64_t(tail[ 6]) << 48;
  case  6: k1 ^= std::uint64_t(tail[ 5]) << 40;
  case  5: k1 ^= std::uint64_t(tail[ 4]) << 32;
  case  4: k1 ^= std::uint64_t(tail[ 3]) << 24;
  case  3: k1 ^= std::uint64_t(tail[ 2]) << 16;
  case  2: k1 ^= std::uint64_t(tail[ 1]) << 8;
  case  1: k1 ^= std::uint64_t(tail[ 0]) << 0;
           k1 *= c1; k1  = ROTL64(k1, 31); k1 *= c2; h1 ^= k1;
  }

  // Finalization.
  h1 ^= len;
  h2 ^= len;

  h1 += h2;
  h2 += h1;

  h1 ^= h1 >> 33;
  h1 *= 0xff51afd7ed558ccd;
  h1 ^= h1 >> 33;
  h1 *= 0xc4ceb9fe1a85ec53;
  h1 ^= h1 >> 33;

  h2 ^= h2 >> 33;
  h2 *= 0xff51afd7ed558ccd;
  h2 ^= h2 >> 33;
  h2 *= 0xc4ceb9fe1a85ec53;
  h2 ^= h2 >> 33;

  h1 += h2;
  h2 += h1;

  return hash_t(h1, h2);
}

}  // namespace vita
#endif  // include guard
