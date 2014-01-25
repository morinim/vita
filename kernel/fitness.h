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

#if !defined(FITNESS_H)
#define      FITNESS_H

#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "kernel/vita.h"

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
    static constexpr T size{N};

    explicit basic_fitness_t(T = std::numeric_limits<T>::lowest());
    template<class... Args> basic_fitness_t(Args...);

    bool operator==(const basic_fitness_t<T, N> &) const;
    bool operator!=(const basic_fitness_t<T, N> &) const;
    bool operator>(const basic_fitness_t<T, N> &) const;
    bool operator>=(const basic_fitness_t<T, N> &) const;
    bool operator<(const basic_fitness_t<T, N> &) const;
    bool operator<=(const basic_fitness_t<T, N> &) const;
    bool dominating(const basic_fitness_t<T, N> &) const;
    bool almost_equal(const basic_fitness_t<T, N> &, T) const;

    T operator[](unsigned i) const { assert(i < N); return vect[i]; }

    T &operator[](unsigned i) { assert(i < N); return vect[i]; }

    bool isfinite() const;
    bool isnan() const;
    bool issmall() const;

    basic_fitness_t &operator+=(const basic_fitness_t &);
    basic_fitness_t &operator-=(const basic_fitness_t &);
    basic_fitness_t operator-(const basic_fitness_t &) const;
    basic_fitness_t operator*(const basic_fitness_t &) const;

    basic_fitness_t operator/(T) const;
    basic_fitness_t operator*(T) const;

    basic_fitness_t abs() const;
    basic_fitness_t sqrt() const;

  public:   // Serialization.
    bool load(std::istream &);
    bool save(std::ostream &) const;

  private:
    std::array<T, N> vect;
  };

  using fitness_t = basic_fitness_t<double, 1>;

#include "kernel/fitness_inl.h"
}  // namespace vita

#endif  // FITNESS_H
