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

#if !defined(VITA_FITNESS_H)
#define      VITA_FITNESS_H

#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "kernel/utility.h"

namespace vita
{
  ///
  /// \brief
  /// A value assigned to an individual which reflects how well the individual
  /// solves the task.
  ///
  /// \tparam N dimension of the fitness vector
  ///
  template<class T, unsigned N>
  class basic_fitness_t
  {
  public:
    typedef T base_t;
    static constexpr decltype(N) size = N;

    explicit basic_fitness_t(T = std::numeric_limits<T>::lowest());
    template<class... Args> basic_fitness_t(Args...);

    bool operator==(const basic_fitness_t &) const;
    bool operator!=(const basic_fitness_t &) const;
    bool operator>(const basic_fitness_t &) const;
    bool operator>=(const basic_fitness_t &) const;
    bool operator<(const basic_fitness_t &) const;
    bool operator<=(const basic_fitness_t &) const;
    bool dominating(const basic_fitness_t &) const;

    T operator[](unsigned) const;
    T &operator[](unsigned);

    basic_fitness_t &operator+=(const basic_fitness_t &);
    basic_fitness_t &operator-=(const basic_fitness_t &);
    basic_fitness_t operator-(basic_fitness_t) const;
    basic_fitness_t operator*(basic_fitness_t) const;

    basic_fitness_t operator/(T) const;
    basic_fitness_t operator*(T) const;

    double distance(const basic_fitness_t &) const;

  public:   // Serialization.
    bool load(std::istream &);
    bool save(std::ostream &) const;

  private:
    std::array<T, N> vect;
  };

  template<class T, unsigned N> bool isfinite(const basic_fitness_t<T, N> &);
  template<class T, unsigned N> bool isnan(const basic_fitness_t<T, N> &);
  template<class T, unsigned N> bool issmall(const basic_fitness_t<T, N> &);

  template<class T, unsigned N>
  basic_fitness_t<T, N> abs(basic_fitness_t<T, N>);
  template<class T, unsigned N>
  basic_fitness_t<T, N> round_to(basic_fitness_t<T, N>);
  template<class T, unsigned N>
  basic_fitness_t<T, N> sqrt(basic_fitness_t<T, N>);

  template<class T> bool almost_equal(T, T, T = 0.00001);
  template<class T, unsigned N> bool almost_equal(
    const basic_fitness_t<T, N> &, const basic_fitness_t<T, N> &, T = 0.00001);

  using fitness_t = basic_fitness_t<double, 1>;

#include "kernel/fitness_inl.h"
}  // namespace vita

#endif  // Include guard
