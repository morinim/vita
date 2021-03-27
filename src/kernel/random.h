/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2021 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_RANDOM_H)
#define      VITA_RANDOM_H

#include <cstdlib>
#include <random>
#include <set>

#include "kernel/common.h"
#include "kernel/range.h"
#include "utility/xoshiro256ss.h"

namespace vita::random
{

enum class distribution {uniform, normal};

///
/// xoshiro256** (XOR/shift/rotate) is an all-purpose, rock-solid generator
/// (not a cryptographically secure generator). It has excellent (sub-ns)
/// speed, a state space (256 bits) that is large enough for any parallel
/// application, and it passes all tests we are aware of.
///
using engine_t = vigna::xoshiro256ss;

extern thread_local engine_t engine;

template<class T> T sup(T);

unsigned ring(unsigned, unsigned, unsigned);

bool boolean(double = 0.5);

void seed(unsigned);
void randomize();

///
/// Used for ephemeral random constant generation.
///
/// \param[in] d  type of distribution
/// \param[in] p1 **minimum** for uniform distribution; **mean** for normal
///               distribution
/// \param[in] p2 **maximum** for uniform distribution, **standard deviation**
///               for normal distribution
/// \return       a random number distributed according to distribution `d`
///
template<class T>
T ephemeral(distribution d, T p1, T p2)
{
  switch (d)
  {
  case distribution::uniform:
    return between(p1, p2);

  case distribution::normal:
    return std::normal_distribution<T>(p1, p2)(engine);
  }
}

///
/// A specialization for floating point values of the `random::between(T, T)`
/// template function.
///
/// \param[in] min minimum random number
/// \param[in] sup upper bound
/// \return        a random `double` in the `[min;sup[` range
///
/// \see
/// For further details:
/// - <http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2013/n3551.pdf>
/// - <http://stackoverflow.com/q/24566574/3235496>
/// - <http://stackoverflow.com/q/25222167/3235496>
///
template<class T>
std::enable_if_t<std::is_floating_point_v<T>, T>
between(T min, T sup)
{
  Expects(min < sup);

  std::uniform_real_distribution<T> d(min, sup);
  return d(engine);
}

///
/// Picks up a random integer value uniformly distributed in the set of
/// integers `{min, min+1, ..., sup-1}`.
///
/// \param[in] min minimum random number
/// \param[in] sup upper bound
/// \return        a random number in the `[min;sup[` range
///
/// \note
/// Contrary to boost usage this function does not take a closed range.
/// Instead it takes a half-open range (C++ usage and same behaviour of the
/// real number distribution).
///
template<class T>
std::enable_if_t<std::is_integral_v<T>, T>
between(T min, T sup)
{
  Expects(min < sup);

  std::uniform_int_distribution<T> d(min, sup - 1);
  return d(engine);
}

template<class T>
std::enable_if_t<std::is_enum_v<T>, T>
between(T min, T sup)
{
  Expects(min < sup);

  return static_cast<T>(between<std::underlying_type_t<T>>(min, sup));
}

///
/// Uniformly extracts a random value in a range.
///
/// \param[in] r a half-open range range
/// \return      a random value in range `r`
///
template<class T>
T in(range_t<T> r)
{
  return between(r.first, r.second);
}

///
/// \param[in] sup upper bound
/// \return        a random number in the [0;sup[ range
///
/// \note This is a shortcut for: `between<T>(0, sup)`
///
template<class T>
T sup(T sup)
{
  return between(static_cast<T>(0), sup);
}

///
/// \param[in] c a STL container
/// \return      a random element of container `c`
///
template<class C>
const typename C::value_type &element(const C &c)
{
  Expects(c.size());

  return *std::next(
    c.begin(),
    static_cast<typename C::difference_type>(sup(c.size())));
}

///
/// \param[in] c a STL container
/// \return      a random element of container `c`
///
template<class C>
typename C::value_type &element(C &c)
{
  Expects(c.size());

  return *std::next(
    c.begin(),
    static_cast<typename C::difference_type>(sup(c.size())));
}

///
/// \param[in] p a probability (`[0;1]` range)
/// \return      `true` `p%` times
///
/// \note `bool` values are produced according to the Bernoulli distribution.
///
inline bool boolean(double p)
{
  Expects(0.0 <= p);
  Expects(p <= 1.0);

  std::bernoulli_distribution d(p);
  return d(engine);

  //return between<double>(0, 1) < p;
}

}  // namespace vita::random

#endif  // include guard
