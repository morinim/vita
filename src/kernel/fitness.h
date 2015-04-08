/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
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
/// Tags used in the `basic_fitness_t`'s constructor.
///
enum class fit_tag {copies_of, components};

///
/// \brief A value assigned to an individual which reflects how well the
///        individual solves the task
///
/// \tparam T a numerical type used as a building block for the fitness
///           (e.g. `double`)
///
/// This is **NOT THE RAW FITNESS**. Raw fitness is stated in the natural
/// terminology of the problem: the better value may be either smaller (as when
/// raw fitness is error) or larger (as when raw fitness is food eaten, benefit
/// achieved...).
///
/// We use a **STANDARDIZED FITNESS**: a greater numerical value is **always**
/// a better value (often the optimal value is 0, but this isn't strictly
/// necessary).
///
/// If, for a particular problem, a greater value of raw fitness is better,
/// standardized fitness equals the raw fitness for that problem (otherwise
/// standardized fitness must be computed from raw fitness).
///
/// \warning
/// The definition of standardized fitness given here is different from that
/// used in Koza's "Genetic Programming: On the Programming of Computers by
/// Means of Natural Selection". In the book a **LOWER** numerical value is
//  always a better one.
/// The main difference is that Vita attempts to maximize the fitness (while
/// other softwares try to minimize it).
/// We chose this convention since it seemed more natural (a greater fitness
/// is a better fitness; achieving a better fitness means to maximize the
/// fitness). The downside is that we have to manage negative numbers, but for
/// out purposes it's not so bad.
/// Anyway maximization and minimization problems are basically the same: the
/// solution of `max(f(x))` is the same as `-min(-f(x))`. This is usually
/// all you have to remember when dealing with examples/problems expressed
/// in the other notation.
///
template<class T>
class basic_fitness_t
{
public:  // Type alias and iterators
  using values_t = std::vector<T>;
  using value_type = typename values_t::value_type;
  using iterator = typename values_t::iterator;
  using const_iterator = typename values_t::const_iterator;

public:
  basic_fitness_t(unsigned, fit_tag, T = std::numeric_limits<T>::lowest());

  basic_fitness_t() : basic_fitness_t(1, fit_tag::components) {}

  basic_fitness_t(std::initializer_list<T>);
  basic_fitness_t(std::vector<T>);

  unsigned size() const;
  T operator[](unsigned) const;
  T &operator[](unsigned);

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  basic_fitness_t &operator+=(const basic_fitness_t &);
  basic_fitness_t &operator-=(const basic_fitness_t &);
  basic_fitness_t &operator*=(const basic_fitness_t &);

  basic_fitness_t operator/(T) const;
  basic_fitness_t operator*(T) const;

public:   // Serialization
  bool load(std::istream &);
  bool save(std::ostream &) const;

private:  // Private data members
  values_t vect_;
};

// ***********************************************************************
// *  Arithmetic operators                                               *
// ***********************************************************************
template<class T>
basic_fitness_t<T> operator+(basic_fitness_t<T>, const basic_fitness_t<T> &);
template<class T>
basic_fitness_t<T> operator-(basic_fitness_t<T>, const basic_fitness_t<T> &);
template<class T>
basic_fitness_t<T> operator*(basic_fitness_t<T>, const basic_fitness_t<T> &);

// ***********************************************************************
// *  Comparison operators                                               *
// ***********************************************************************
template<class T> bool operator==(const basic_fitness_t<T> &,
                                  const basic_fitness_t<T> &);
template<class T> bool operator!=(const basic_fitness_t<T> &,
                                  const basic_fitness_t<T> &);
template<class T> bool operator>(const basic_fitness_t<T> &,
                                 const basic_fitness_t<T> &);
template<class T> bool operator<(const basic_fitness_t<T> &,
                                 const basic_fitness_t<T> &);
template<class T> bool operator>=(const basic_fitness_t<T> &,
                                  const basic_fitness_t<T> &);
template<class T> bool operator<=(const basic_fitness_t<T> &,
                                  const basic_fitness_t<T> &);

template<class T> bool almost_equal(const basic_fitness_t<T> &,
                                    const basic_fitness_t<T> &, T = 0.00001);
template<class T> bool dominating(const basic_fitness_t<T> &,
                                  const basic_fitness_t<T> &);

// ***********************************************************************
// *  Other functions                                                    *
// ***********************************************************************
template<class T> bool isfinite(const basic_fitness_t<T> &);
template<class T> bool isnan(const basic_fitness_t<T> &);
template<class T> bool isnonnegative(const basic_fitness_t<T> &);
template<class T> bool issmall(const basic_fitness_t<T> &);

template<class T> basic_fitness_t<T> abs(basic_fitness_t<T>);
template<class T> basic_fitness_t<T> combine(const basic_fitness_t<T> &,
                                             const basic_fitness_t<T> &);
template<class T> double distance(const basic_fitness_t<T> &,
                                  const basic_fitness_t<T> &);
template<class T> basic_fitness_t<T> round_to(basic_fitness_t<T>);
template<class T> basic_fitness_t<T> sqrt(basic_fitness_t<T>);

using fitness_t = basic_fitness_t<double>;

#include "kernel/fitness.tcc"
}  // namespace vita

#endif  // Include guard
