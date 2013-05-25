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

#include <fstream>
#include <vector>

#include "environment.h"

namespace vita
{
  ///
  /// \brief A group of individual which may interact together (for example by
  ///        mating) producing offspring.
  ///
  /// Typical population size in GP ranges from ten to many thousands.
  ///
  template<class T>
  class population
  {
  public:
    typedef typename std::vector<T>::const_iterator const_iterator;

    explicit population(const environment &);

    T &operator[](size_t);
    const T &operator[](size_t) const;

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
    std::vector<T> pop_;
  };

#include "population_inl.h"

  ///
  /// \example example2.cc
  /// Creates a random population and shows its content.
  ///
}  // namespace vita

#endif  // POPULATION_H
