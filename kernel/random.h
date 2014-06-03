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

namespace vita { namespace random
{
  enum class distribution {uniform, normal};

  template<class T> T between(T, T);
  template<class T> T sup(T);

  template<class C> const typename C::value_type &element(const C &);
  template<class C> typename C::value_type &element(C &);

  unsigned ring(unsigned, unsigned, unsigned);

  bool boolean(double = 0.5);

  void seed(unsigned);
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

  ///
  /// \return a reference to a single engine shared whereever needed.
  ///
  /// This function grants access to a shared engine.
  ///
  /// \note
  /// \c the engine can be shared among multiple threads.
  ///
  inline engine_t &engine()
  {
    // We are using a *global* generator object here. This is important because
    // we don't want to create a new pseudo-random number generator at every
    // call.
    // The numbers produced will be the same every time the program is run.
    thread_local engine_t e{28071973u};  // Magic!!!

    return e;
  }

  ///
  /// \param[in] d type of distribution.
  /// \param[in] p1 minimum for uniform distribution, mean for normal
  ///               distribution.
  /// \param[in] p2 maximum for uniform distribution, standard deviation for
  ///               normal distribution.
  /// \return a random number distributed according to \a d distribution.
  ///
  /// This is used for ephemeral random constant generation.
  ///
  template<class T>
  T ephemeral(distribution d, T p1, T p2)
  {
    switch (d)
    {
    case uniform:
      return between(p1, p2);

    case normal:
      return std::normal_distribution<T>(p1, p2)(engine());
    }
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
  inline double between(double min, double sup)
  {
    assert(min < sup);

    std::uniform_real_distribution<double> d(min, sup);

    return d(engine());
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
  T between(T min, T sup)
  {
    static_assert(std::is_integral<T>::value,
                  "random::between needs a template-parameter of integer type");

    assert(min < sup);

    std::uniform_int_distribution<T> d(min, sup - 1);

    return d(engine());
  }

  ///
  /// \param[in] sup upper bound.
  /// \return a random number in the [0;sup[ range.
  ///
  /// \note
  /// This is a shortcut for: \c between<T>(0, sup);
  ///
  template<class T>
  T sup(T sup)
  {
    return between<T>(0, sup);
  }

  ///
  /// \param[in] c a STL container.
  /// \return a random element of container \a c.
  ///
  template<class C>
  const typename C::value_type &element(const C &c)
  {
    const auto size(c.size());
    assert(size);

    return *std::next(c.begin(),
                      static_cast<typename C::difference_type>(
                        between<decltype(size)>(0, size)));
  }

  ///
  /// \param[in] c a STL container.
  /// \return a random element of container \a c.
  ///
  template<class C>
  typename C::value_type &element(C &c)
  {
    const auto size(c.size());
    assert(size);

    return *std::next(c.begin(),
                      static_cast<typename C::difference_type>(
                        between<decltype(size)>(0, size)));
  }

  ///
  /// \param[in] p a probability ([0;1] range).
  /// \return \c true \a p% times.
  ///
  /// bool values are produced according to the Bernoulli distribution.
  ///
  inline bool boolean(double p)
  {
    assert(0.0 <= p && p <= 1.0);

    std::bernoulli_distribution d(p);
    return d(engine());

    //return between<double>(0, 1) < p;
  }
}}  // namespace vita::random

#endif  // Include guard
