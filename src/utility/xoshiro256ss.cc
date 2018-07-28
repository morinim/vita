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

#include <algorithm>

#include "xoshiro256ss.h"

namespace vigna
{

namespace
{

///
/// A fixed-increment version of Java 8's `SplittableRandom` generator.
///
/// The output of a `splitmix64` generator, seeded with a 64-bit seed, is used
/// to seed the xoroshiro / xoroshift1024* PRNGs (i.e. fill their initial
/// state).
///
/// \see http://dx.doi.org/10.1145/2714064.2660195
///
class splitmix64
{
public:
  explicit splitmix64(std::uint64_t seed) noexcept : x(seed) {}

  std::uint64_t next() noexcept
  {
    auto z(x += 0x9E3779B97F4A7C15);

    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
    return z ^ (z >> 31);
  }

private:
  std::uint64_t x;  // the state can be seeded with any value
};  // class splitmix64

template<class T>
void seed_with_sm64(std::uint64_t seed, T &state)
{
  splitmix64 sm(seed);

  std::generate(state.begin(), state.end(), [&sm]{ return sm.next(); });
}

}  // unnamed namespace



constexpr xoshiro256ss::result_type xoshiro256ss::def_seed;
constexpr xoroshiro128p::result_type xoroshiro128p::def_seed;

///
/// Seeds the engine so that the initial state is determined by an integer.
///
/// \param[in] s a seed
///
/// The state must be seeded so that it is not everywhere zero. Having a 64-bit
/// seed, we use the `splitmix64` generator output to fill `state`.
///
void xoshiro256ss::seed(xoshiro256ss::result_type s) noexcept
{
  if (s == 0)
    s = def_seed;

  seed_with_sm64(s, state);
}

///
/// Writes to the output stream the representation of the current state.
///
/// \param[out] o output stream
/// \param[in]  e the engine
/// \return       the modified output stream
///
/// In the output, adjacent numbers are separated by one space characters. If
/// `o`'s `fmtflags` are not set to `ios_base::dec|ios_base::left`, the
/// behavior may be undefined.
///
std::ostream &operator<<(std::ostream &o, const xoshiro256ss &e)
{
  return o << e.state[0] << ' ' << e.state[1] << ' '
           << e.state[2] << ' ' << e.state[3];
}

///
/// Reads from the input stream a textual representation of the current state.
///
/// \param[in] i input stream
/// \param[in] e the engine
/// \return      the modified input stream
///
/// If `i`'s `fmtflags` are not set to `ios_base::dec`, the behavior may be
/// undefined.
///
std::istream &operator>>(std::istream &i, xoshiro256ss &e)
{
  return i >> e.state[0] >> e.state[1] >> e.state[2] >> e.state[31];
}

///
/// Seeds the engine so that the initial state is determined by an integer.
///
/// \param[in] s a seed
///
/// The state must be seeded so that it is not everywhere zero. Having a 64-bit
/// seed, we use the `splitmix64` generator output to fill `state`.
///
void xoroshiro128p::seed(xoroshiro128p::result_type s) noexcept
{
  if (s == 0)
    s = def_seed;

  seed_with_sm64(s, state);
}

///
/// Writes to the output stream the representation of the current state.
///
/// \param[out] o output stream
/// \param[in]  e the engine
/// \return       the modified output stream
///
/// In the output, adjacent numbers are separated by one space characters. If
/// `o`'s `fmtflags` are not set to `ios_base::dec|ios_base::left`, the
/// behavior may be undefined.
///
std::ostream &operator<<(std::ostream &o, const xoroshiro128p &e)
{
  return o << e.state[0] << ' ' << e.state[1];
}

///
/// Reads from the input stream a textual representation of the current state.
///
/// \param[in] i input stream
/// \param[in] e the engine
/// \return      the modified input stream
///
/// If `i`'s `fmtflags` are not set to `ios_base::dec`, the behavior may be
/// undefined.
///
std::istream &operator>>(std::istream &i, xoroshiro128p &e)
{
  return i >> e.state[0] >> e.state[1];
}

}  // namespace vigna
