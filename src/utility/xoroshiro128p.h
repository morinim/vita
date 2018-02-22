// Original and permanent link: http://xoroshiro.di.unimi.it/
// Written in 2016 by David Blackman and Sebastiano Vigna (vigna@acm.org)
//
// To the extent possible under law, the author has dedicated all copyright
// and related and neighboring rights to this software to the public domain
// worldwide. This software is distributed without any warranty.
//
// See <http://creativecommons.org/publicdomain/zero/1.0/>.

/**
 *  \file
 *
 *  \copyright Copyright (C) 2018 Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see http://xoroshiro.di.unimi.it/
 */

#if !defined(PRNG_XOROSHIRO_H)
#define      PRNG_XOROSHIRO_H

#include <array>
#include <iostream>
#include <limits>
#include <random>

namespace vigna
{

///
/// The successor to xorshift128+.
///
/// It is the fastest full-period generator passing BigCrush without systematic
/// failures, but due to the relatively short period it is acceptable only for
/// applications with a mild amount of parallelism; otherwise, use a
/// xorshift1024* generator.
///
/// Beside passing BigCrush, this generator passes the PractRand test suite up
/// to (and included) 16TB, with the exception of binary rank tests, as the
/// lowest bit of this generator is an LFSR of degree 128. The next bit can be
/// described by an LFSR of degree 8256, but in the long run it will fail
/// linearity tests, too. The other bits needs a much higher degree to be
/// represented as LFSRs.
///
/// We suggest to use a sign test to extract a random `bool` value and right
/// shifts to extract subsets of bits.
///
class xoroshiro128p
{
public:
  using result_type = std::uint64_t;

  // If one doesn't specify a seed for the PRNG, it uses a default one.
  explicit xoroshiro128p(result_type s = def_seed) noexcept : state()
  { seed(s); }

  /// \return the smallest value that `operator()` may return. The value is
  ///         strictly less than `max()`
  constexpr static result_type min() noexcept { return 0; }

  /// \return the largest value that `operator()` may return. The value is
  //          strictly greater than `min()`
  constexpr static result_type max() noexcept
  { return std::numeric_limits<result_type>::max(); }

  /// \return a value in the closed interval `[min(), max()]`. Has amortized
  ///         constant complexity
  result_type operator()() noexcept
  {
    const auto s0(state[0]);
    auto s1(state[1]);
    const auto result(s0 + s1);

    s1 ^= s0;
    state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14);  // a, b
    state[1] = rotl(s1, 36);                    // c

    return result;
  }

  void seed() noexcept ;
  void seed(result_type) noexcept;

  bool operator==(const xoroshiro128p &rhs) const noexcept
  { return state == rhs.state; }
  bool operator!=(const xoroshiro128p &rhs) const noexcept
  { return !(*this == rhs); }

  friend std::ostream &operator<<(std::ostream &, const xoroshiro128p &);
  friend std::istream &operator>>(std::istream &, xoroshiro128p &);

private:
  static constexpr result_type def_seed = 0xcced1fc561884152;

  // Most compilers will turn this simulated rotate operation into a single
  // instruction.
  static constexpr std::uint64_t rotl(std::uint64_t x, int k) noexcept
  {
    return (x << k) | (x >> (64 - k));
  }

  std::array<std::uint64_t, 2> state;
};  // class xoroshiro128p

}  // namespace vigna

#endif  // include guard
