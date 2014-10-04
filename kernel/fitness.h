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

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

#include "kernel/utility.h"

namespace vita
{
  ///
  /// \brief A value assigned to an individual which reflects how well the
  ///        individual solves the task
  ///
  template<class T>
  class basic_fitness_t
  {
  public:
    using value_type = T;

    explicit basic_fitness_t(unsigned = 1,
                             T = std::numeric_limits<T>::lowest());
    template<class... Args> explicit basic_fitness_t(Args...);

    bool operator==(const basic_fitness_t &) const;
    bool operator!=(const basic_fitness_t &) const;
    bool operator>(const basic_fitness_t &) const;
    bool operator>=(const basic_fitness_t &) const;
    bool operator<(const basic_fitness_t &) const;
    bool operator<=(const basic_fitness_t &) const;
    bool dominating(const basic_fitness_t &) const;

    unsigned size() const;
    T operator[](unsigned) const;
    T &operator[](unsigned);

    basic_fitness_t &operator+=(const basic_fitness_t &);
    basic_fitness_t &operator-=(const basic_fitness_t &);
    basic_fitness_t operator-(basic_fitness_t) const;
    basic_fitness_t operator*(basic_fitness_t) const;

    basic_fitness_t operator/(T) const;
    basic_fitness_t operator*(T) const;

    double distance(const basic_fitness_t &) const;

  public:   // Serialization
    bool load(std::istream &);
    bool save(std::ostream &) const;

  private:
    std::vector<T> vect_;
  };

  template<class T> bool isfinite(const basic_fitness_t<T> &);
  template<class T> bool isnan(const basic_fitness_t<T> &);
  template<class T> bool isnonnegative(const basic_fitness_t<T> &);
  template<class T> bool issmall(const basic_fitness_t<T> &);

  template<class T> basic_fitness_t<T> abs(basic_fitness_t<T>);
  template<class T> basic_fitness_t<T> round_to(basic_fitness_t<T>);
  template<class T> basic_fitness_t<T> sqrt(basic_fitness_t<T>);

  template<class T> bool almost_equal(const basic_fitness_t<T> &,
                                      const basic_fitness_t<T> &, T = 0.00001);

  using fitness_t = basic_fitness_t<double>;

#include "kernel/fitness_inl.h"
}  // namespace vita

#endif  // Include guard
