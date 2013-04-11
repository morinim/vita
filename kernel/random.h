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

#include <boost/random.hpp>

#include "vita.h"

namespace vita
{
  class random
  {
  public:
    template<class T> static T between(T, T);
    template<class T> static const T &element(const std::vector<T> &);

    static size_t ring(size_t, size_t, size_t);

    static bool boolean(double);
    static bool boolean();

    static void seed(unsigned);

  private:
    // This generator produces integers in the range [0, 2^32-1] with a good
    // uniform distribution in up to 623 dimensions.
    typedef boost::mt19937 base_gen;

    // We are using a *global* generator object here. This is important because
    // we don't want to create a new pseudo-random number generator at every
    // call.
    // The numbers produced will be the same every time the program is run.
    static base_gen rng_;
  };

  ///
  /// \param[in] min minimum random number.
  /// \param[in] sup upper bound.
  /// \return a random \c double in the [min;sup[ range.
  ///
  /// This is a specialization for double values of the \c random::between(T, T)
  /// template function. The main difference is that here we use
  /// \c uniform_real<> distribution instead of \c uniform_int<>.
  ///
  template<>
  inline
  double random::between(double min, double sup)
  {
    assert(min < sup);

    return boost::uniform_real<>(min, sup)(rng_);
  }

  ///
  /// \param[in] min minimum random number.
  /// \param[in] sup upper bound.
  /// \return a random number in the [min;sup[ range.
  ///
  /// Picks up a random integer value uniformly distributed in the set of
  /// integers {min, min+1, ..., sup-1}.
  /// Please note that contrary to boost usage this function does not take a
  /// closed range. Instead it takes a half-open range (C++ usage and same
  /// behaviour of the real number distribution).
  ///
  template<class T>
  inline
  T random::between(T min, T sup)
  {
    assert(min < sup);

    return boost::uniform_int<>(min, sup - 1)(rng_);
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
    return vect[between<unsigned>(0, vect.size())];
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
    //return boost::uniform_01<double>()(rng_) < p;
  }

  ///
  /// \return \c true 50% times.
  ///
  inline
  bool random::boolean()
  {
    return random::between<unsigned>(0, 2) != 0;
    //return boost::uniform_smallint<unsigned>(0, 1)(rng_) != 0;
  }
}  // namespace vita

#endif  // RANDOM_H
