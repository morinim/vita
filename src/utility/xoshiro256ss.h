// Original and permanent links:
// - http://xoroshiro.di.unimi.it/
// - http://xoshiro.di.unimi.it/
//
// Written in 2016-2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)
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
 *  \see http://xoshiro.di.unimi.it/
 */

#if !defined(PRNG_XOSHIRO_H)
#define      PRNG_XOSHIRO_H

#include <array>
#include <iostream>
#include <limits>
#include <random>

///
/// The main 64-bit proposal for an all-purpose, rock-solid generator is
/// xoshiro256**. It has excellent speed, a state space that is large enough
/// for any parallel application, and passes all tests we are aware of. While
/// specified using multiplications, it can be implemented using only a few
/// shifts, xors and sums (rotations can be implemented with shifts, when not
/// directly available).
///
/// There are however some cases in which 256 bits of state are considered too
/// much, for instance when throwing a very large number of lightweight
/// threads, or in embedded hardware. In this case a similar discussion applies
/// to xoroshiro128+, with the caveat that the latter has mild problems with
/// the Hamming-weight dependency test: however, bias can be detected only
/// after 8TB of data, which makes it unlikely to affect in any way
/// applications.
///
namespace vigna
{

// Most compilers will turn this simulated rotate operation into a single
// instruction.
constexpr std::uint64_t rotl(std::uint64_t x, int k) noexcept
{
  return (x << k) | (x >> (64 - k));
}

///
/// xoshiro256** v1.0, an all-purpose, rock-solid generator.
///
/// It has excellent (sub-ns) speed, a state (256 bits) that is large enough
/// for any parallel application, and it passes all tests we are aware of.
/// For generating just floating-point numbers, xoshiro256+ is even faster.
/// The state must be seeded so that it is not everywhere zero.
///
/// \note
/// If you have a 64-bit seed, we suggest to seed a `splitmix64` generator and
/// use its output to fill the seed (research has shown that initialization
/// must be performed with a generator radically different in nature from the
/// one initialized to avoid correlation on similar seeds).
///
class xoshiro256ss
{
public:
  using result_type = std::uint64_t;

  // If one doesn't specify a seed for the PRNG, it uses a default one.
  explicit xoshiro256ss(result_type s = def_seed) noexcept : state()
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
    const auto result_starstar(rotl(state[1] * 5, 7) * 9);
    const auto t(state[1] << 17);

    state[2] ^= state[0];
    state[3] ^= state[1];
    state[1] ^= state[2];
    state[0] ^= state[3];

    state[2] ^= t;

    state[3] = rotl(state[3], 45);

	return result_starstar;
  }

  void seed() noexcept ;
  void seed(result_type) noexcept;

  bool operator==(const xoshiro256ss &rhs) const noexcept
  { return state == rhs.state; }
  bool operator!=(const xoshiro256ss &rhs) const noexcept
  { return !(*this == rhs); }

  friend std::ostream &operator<<(std::ostream &, const xoshiro256ss &);
  friend std::istream &operator>>(std::istream &, xoshiro256ss &);

private:
  static constexpr result_type def_seed = 0xcced1fc561884152;

  std::array<std::uint64_t, 4> state;
};  // class xoshiro256ss


///
/// xoroshiro128+, an all-purpose, rock-solid generator.
///
/// It's the fastest full-period generator passing BigCrush without systematic
/// failures, but due to the relatively short period it is acceptable only for
/// applications with a mild amount of parallelism.
///
/// It passes all tests we are aware of except for the four lower bits, which
/// might fail linearity tests (and just those), so if low linear complexity is
/// not considered an issue (as it is usually the case) it can be used to
/// generate 64-bit outputs, too; moreover, this generator has a very mild
/// Hamming-weight dependency making our test (http://prng.di.unimi.it/hwd.php)
/// fail after 8 TB of output; we believe this slight bias cannot affect any
/// application. If you are concerned, use xoroshiro128** or xoshiro256+.
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
    state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);  // a, b
    state[1] = rotl(s1, 37);                    // c

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

  std::array<std::uint64_t, 2> state;
};  // class xoroshiro128p

}  // namespace vigna

#endif  // include guard
