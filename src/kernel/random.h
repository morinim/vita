/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2018 EOS di Manlio Morini.
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
#include "utility/xoroshiro128p.h"

namespace vita
{

///
/// Half closed interval.
///
/// `range{m, u}` specifies the half closed interval `[m; u[` (`m` is the
/// minimum value, `u` an exclusive thight upper bound).
///
template<class T> using range_t = std::pair<T, T>;

template<class T1, class T2>
constexpr std::pair<T1, T2> range(T1 &&t, T2 &&u)
{ return std::pair<T1, T2>(std::forward<T1>(t), std::forward<T2>(u)); }


namespace random
{

enum class distribution {uniform, normal};

///
/// xoroshiro128+ (XOR/rotate/shift/rotate) is the successor to xorshift128+.
/// Instead of perpetuating Marsaglia's tradition of xorshift as a basic
/// operation, xoroshiro128+ uses a carefully handcrafted shift/rotate-based
/// linear transformation designed in collaboration with David Blackman. The
/// result is a significant improvement in speed (well below a nanosecond per
/// integer) and a significant improvement in statistical quality, as detected
/// by the long-range tests of PractRand. xoroshiro128+ is our current
/// suggestion for replacing low-quality generators commonly found in
/// programming languages. It is the default generator in Erlang.
///
using engine_t = vigna::xoroshiro128p;

extern thread_local engine_t engine;

template<class T> T sup(T);

template<class C> const typename C::value_type &element(const C &);
template<class C> typename C::value_type &element(C &);

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
std::enable_if_t<std::is_floating_point<T>::value, T>
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
std::enable_if_t<std::is_integral<T>::value, T>
between(T min, T sup)
{
  Expects(min < sup);

  std::uniform_int_distribution<T> d(min, sup - 1);
  return d(engine);
}

template<class T>
std::enable_if_t<std::is_enum<T>::value, T>
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
  const auto size(c.size());
  Expects(size);

  return *std::next(c.begin(), sup<typename C::difference_type>(size));
}

///
/// \param[in] c a STL container
/// \return      a random element of container `c`
///
template<class C>
typename C::value_type &element(C &c)
{
  const auto size(c.size());
  Expects(size);

  return *std::next(c.begin(),
                    static_cast<typename C::difference_type>(sup(size)));
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

}  // namespace random
}  // namespace vita

#endif  // include guard
