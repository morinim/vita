/**
 *
 *  \file random.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if !defined(RANDOM_H)
#define      RANDOM_H

#include <boost/random.hpp>

#include <cstdlib>

#include "kernel/vita.h"

namespace vita
{
  class random
  {
  public:
    template<class T> static T between(T, T);
    template<class T> static const T &element(const std::vector<T> &);

    static index_t ring(index_t, unsigned, unsigned);

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

    return boost::uniform_int<>(min, sup-1)(rng_);
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
    assert(0 <= p && p <= 1);

    return random::between<double>(0, 1) < p;
  }

  ///
  /// \return \c true 50% times.
  ///
  inline
  bool random::boolean()
  {
    return random::between<unsigned>(0, 2) != 0;
  }
}  // namespace vita

#endif  // RANDOM_H
