/**
 *
 *  \file random.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(RANDOM_H)
#define      RANDOM_H

#include <cstdlib>
#include <random>
#include <set>

#include "vita.h"

namespace vita
{
  class random
  {
  public:
    // This engine produces integers in the range [0, 2^32-1] with a good
    // uniform distribution in up to 623 dimensions.
    typedef std::mt19937 engine_t;

    template<class T> static T between(T, T);
    template<class T> static const T &element(const std::vector<T> &);
    template<class T> static const T &element(const std::set<T> &);

    static size_t ring(size_t, size_t, size_t);

    static bool boolean(double);
    static bool boolean();

    static void seed(unsigned);
    void randomize();

  private:
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

    //return boost::uniform_real<>(min, sup)(engine());
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
    //return boost::uniform_int<>(min, sup - 1)(engine());
  }

  ///
  /// \param[in] vect a vector.
  /// \return a random element of the vector \a vect.
  ///
  template<class T>
  inline
  const T &random::element(const std::vector<T> &vect)
  {
    assert(vect.size());
    return vect[between<size_t>(0, vect.size())];
  }

  ///
  /// \param[in] s a set.
  /// \return a random element of the set \a s.
  ///
  template<class T>
  inline
  const T &random::element(const std::set<T> &s)
  {
    assert(s.size());

    auto it(s.cbegin());
    std::advance(it, between<size_t>(0, s.size()));

    return *it;
  }

  ///
  /// \param[in] p a probability ([0;1] range).
  /// \return \c true \a p% times.
  ///
  inline
  bool random::boolean(double p)
  {
    assert(0.0 <= p && p <= 1.0);

    return random::between<double>(0, 1) < p;
    //return boost::uniform_01<double>()(engine()) < p;
  }

  ///
  /// \return \c true 50% times.
  ///
  inline
  bool random::boolean()
  {
    return random::between<unsigned>(0, 2) != 0;
    //return boost::uniform_smallint<unsigned>(0, 1)(engine()) != 0;
  }
}  // namespace vita

#endif  // RANDOM_H
