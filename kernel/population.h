/**
 *
 *  \file population.h
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

#if !defined(POPULATION_H)
#define      POPULATION_H

#include <vector>

#include "kernel/vita.h"
#include "kernel/analyzer.h"
#include "kernel/individual.h"

namespace vita
{
  class environment;

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

    explicit population(environment *const);
    void build();

    individual &operator[](size_type);
    const individual &operator[](size_type) const;
    size_type size() const;

    const environment &env() const;

    bool check() const;

  private:
    environment            *env_;
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
  /// \example example2.cc
  /// Creates a random population and shows its content.
  ///
}  // namespace vita

#endif  // POPULATION_H
