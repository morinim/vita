/**
 *
 *  \file population.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(POPULATION_H)
#define      POPULATION_H

#include <vector>

#include "analyzer.h"
#include "environment.h"
#include "individual.h"

namespace vita
{
  ///
  /// \brief A group of individual which may interact together (for example by
  ///        mating) producing offspring.
  ///
  /// Typical population size in GP ranges from ten to many thousands.
  ///
  class population
  {
  public:
    typedef std::vector<individual>::const_iterator const_iterator;

    explicit population(const environment &);

    individual &operator[](size_t);
    const individual &operator[](size_t) const;

    const_iterator begin() const;
    const_iterator end() const;

    size_t individuals() const;

    void inc_age();

    const environment &env() const;

    bool debug(bool) const;

  public:   // Serialization.
    bool load(std::istream &);
    bool save(std::ostream &) const;

  private:  // Private support methods.
    void clear(const environment &, size_t);

  private:  // Private data members.
    std::vector<individual> pop_;
  };

  std::ostream &operator<<(std::ostream &, const population &);

  ///
  /// \param[in] i index of an \a individual.
  /// \return a reference to the \a individual at index \a i.
  ///
  inline
  individual &population::operator[](size_t i)
  {
    assert(i < individuals());
    return pop_[i];
  }

  ///
  /// \param[in] i index of an individual.
  /// \return a constant reference to the individual at index \a i.
  ///
  inline
  const individual &population::operator[](size_t i) const
  {
    assert(i < individuals());
    return pop_[i];
  }

  ///
  /// \return the number of individuals in the population.
  ///
  inline
  size_t population::individuals() const
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

  inline
  population::const_iterator population::begin() const
  {
    return pop_.begin();
  }

  inline
  population::const_iterator population::end() const
  {
    return pop_.end();
  }

  ///
  /// \example example2.cc
  /// Creates a random population and shows its content.
  ///
}  // namespace vita

#endif  // POPULATION_H
