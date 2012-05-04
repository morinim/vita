/**
 *
 *  \file population.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(POPULATION_H)
#define      POPULATION_H

#include <vector>

#include "kernel/vita.h"
#include "kernel/analyzer.h"
#include "kernel/environment.h"
#include "kernel/individual.h"

namespace vita
{
  ///
  /// A group of individual which may interact together (for example by mating)
  /// producing offspring. Typical population size in GP range from ten to
  /// many thousands.
  ///
  class population
  {
  public:
    typedef individual value_type;
    typedef std::vector<value_type>::size_type size_type;

    explicit population(const environment &);

    individual &operator[](size_type);
    const individual &operator[](size_type) const;
    size_type size() const;

    const environment &env() const;

    bool check() const;

  private:
    std::vector<value_type> pop_;
  };

  std::ostream &operator<<(std::ostream &, const population &);

  ///
  /// \param[in] i index of an \a individual (in the [0,population size[ range).
  /// \return a reference to the \a individual at index \a i.
  ///
  inline
  individual &population::operator[](size_type i)
  {
    assert(i < pop_.size());
    return pop_[i];
  }

  ///
  /// \param[in] i index of an individual (int the [0,population size[ range).
  /// \return a constant reference to the individual at index \a i.
  ///
  inline
  const individual &population::operator[](size_type i) const
  {
    assert(i < pop_.size());
    return pop_[i];
  }

  ///
  /// \return the number of individuals in the population.
  ///
  inline
  population::size_type population::size() const
  {
    return pop_.size();
  }

  ///
  /// \return a constant reference to the active environment.
  ///
  inline
  const environment &population::env() const
  {
    return pop_[0].env();
  }

  ///
  /// \example example2.cc
  /// Creates a random population and shows its content.
  ///
}  // namespace vita

#endif  // POPULATION_H
