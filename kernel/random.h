/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
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

#include "kernel/vita.h"

namespace vita
{
  class random
  {
  public:
    template<class T> static T between(T, T);
    template<class T> static T sup(T);

    template<class T> static const T &element(const std::vector<T> &);
    template<class T> static T &element(std::vector<T> &);
    template<class T> static const T &element(const std::set<T> &);

    static unsigned ring(unsigned, unsigned, unsigned);

    static bool boolean(double = 0.5);

    static void seed(unsigned);
    void randomize();

    // The Mersenne Twister engine produces integers with a good
    // uniform distribution.
    // std::mt19937 and std::mt19937_64 are similar. There aren't
    // memory consumption differences and speed is almost equal.
    // Warning: checking for sizeof(int) <= 4 is perhaps not a very
    // portable way since on a 64-bit Windows machine, GCC (MinGW)
    // x64 compiler gives sizeof(int) = sizeof(long) = 4. So we use
    // void *.
    // using engine_t = std::conditional<sizeof(void *) <= 4,
    //                                   std::mt19937,
    //                                   std::mt19937_64>::type;
    using engine_t = std::mt19937;

    static engine_t &engine();
  };

  ///
  /// \return a reference to a single engine shared whereever needed.
  ///
  /// This function grants access to a shared engine.
  ///
  /// \note
  /// \c engine() must be enhanced with appropriate synchronization if the
  /// engine is to be shared among multiple threads.
  ///
  inline random::engine_t &random::engine()
  {
    // We are using a *global* generator object here. This is important because
    // we don't want to create a new pseudo-random number generator at every
    // call.
    // The numbers produced will be the same every time the program is run.
    static engine_t e{28071973u};  // Magic!!!

    return e;
  }

  ///
  /// \param[in] min minimum random number.
  /// \param[in] sup upper bound.
  /// \return a random \c double in the [min;sup[ range.
  ///
  /// This is a specialization for double values of the \c random::between(T, T)
  /// template function. The main difference is that here we use
  /// \c uniform_real<> distribution instead of \c uniform_int<>.
  ///
  /// \see <http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2013/n3551.pdf>
  /// for further details.
  ///
  template<>
  inline
  double random::between(double min, double sup)
  {
    assert(min < sup);

    static std::uniform_real_distribution<> d{};

    using parm_t = decltype(d)::param_type;
    return d(engine(), parm_t{min, sup});
  }

  ///
  /// \param[in] min minimum random number.
  /// \param[in] sup upper bound.
  /// \return a random number in the [min;sup[ range.
  ///
  /// Picks up a random integer value uniformly distributed in the set of
  /// integers {min, min+1, ..., sup-1}.
  ///
  /// \note
  /// Contrary to boost usage this function does not take a closed range.
  /// Instead it takes a half-open range (C++ usage and same behaviour of the
  /// real number distribution).
  ///
  template<class T>
  inline
  T random::between(T min, T sup)
  {
    assert(min < sup);

    static std::uniform_int_distribution<> d{};
    using parm_t = decltype(d)::param_type;

    return d(engine(), parm_t(min, sup - 1));
  }

  ///
  /// \param[in] sup upper bound.
  /// \return a random number in the [0;sup[ range.
  ///
  /// \note
  /// This is a shortcut for: \c between<T>(0, sup);
  ///
  template<class T>
  inline
  T random::sup(T sup)
  {
    return between<T>(T(0), sup);
  }

  ///
  /// \param[in] v a vector.
  /// \return a random element of vector \a v.
  ///
  template<class T>
  inline
  const T &random::element(const std::vector<T> &v)
  {
    const auto size(v.size());
    assert(size);
    return v[between<decltype(size)>(0, size)];
  }

  template<class T>
  inline
  T &random::element(std::vector<T> &v)
  {
    const auto size(v.size());
    assert(size);
    return v[between<decltype(size)>(0, size)];
  }

  ///
  /// \param[in] s a set.
  /// \return a random element of the set \a s.
  ///
  template<class T>
  inline
  const T &random::element(const std::set<T> &s)
  {
    const auto size(s.size());
    assert(size);

    auto it(s.cbegin());
    std::advance(it, between<decltype(size)>(0, size));

    return *it;
  }

  ///
  /// \param[in] p a probability ([0;1] range).
  /// \return \c true \a p% times.
  ///
  /// bool values are produced according to the Bernoulli distribution.
  ///
  inline
  bool random::boolean(double p)
  {
    assert(0.0 <= p && p <= 1.0);

    static std::bernoulli_distribution d{};

    using parm_t = decltype(d)::param_type;
    return d(engine(), parm_t{p});

    //return random::between<double>(0, 1) < p;
  }
}  // namespace vita

#endif  // Include guard
