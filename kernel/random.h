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

    static unsigned ring(unsigned, unsigned, unsigned);

    static bool boolean(double);
    static bool boolean();

    static void seed(unsigned);

  private:
    typedef boost::mt19937 base_gen;

    static base_gen rng_;
  };

  ///
  /// \param[in] min minimum random number.
  /// \param[in] sup upper bound.
  /// \return a random number in the [min;sup[ range.
  ///
  template<>
  inline
  double random::between(double min, double sup)
  {
    assert(min < sup);

    typedef boost::uniform_real<> dist_t;
    typedef boost::variate_generator<base_gen &, dist_t> gen_t;

    gen_t die(rng_, dist_t(min, sup));
    const double ret(die());
    assert(min <= ret && ret < sup);

    return ret;
  }

  template<class T>
  inline
  T random::between(T min, T sup)
  {
    assert(min < sup);

    typedef boost::uniform_int<> dist_t;
    typedef boost::variate_generator<base_gen &, dist_t> gen_t;

    gen_t die(rng_, dist_t(min, sup-1));
    const T ret(die());
    assert(min <= ret && ret < sup);

    return ret;
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
