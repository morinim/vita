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
  /// \brief The coordinates of an individual in the population.
  ///
  /// Population can be organized on multiple layers so we need something
  /// more advanced than a simple index.
  ///
  class coord
  {
  public:
    coord() {}

    /// \param[in] l layer.
    /// \param[in] i index in the layer.
    coord(size_t l, size_t i) : layer(l), index(i) {}

    std::uint32_t layer;
    std::uint32_t index;
  };

  ///
  /// \brief A group of individual which may interact together (for example by
  ///        mating) producing offspring.
  ///
  /// Typical population size in GP ranges from ten to many thousands.
  ///
  class population
  {
  public:
    explicit population(const environment &);

    individual &operator[](const coord &);
    const individual &operator[](const coord &) const;

    size_t layers() const;
    size_t individuals(size_t) const;
    size_t individuals() const;

    bool aged(const coord &) const;
    void inc_age();
    unsigned max_age(size_t) const;

    void reset_layer();

    const environment &env() const;

    bool debug(bool) const;

  public:   // Serialization.
    bool load(std::istream &);
    bool save(std::ostream &) const;

  private:  // Private support methods.
    void clear(const environment &, size_t);

  private:  // Private data members.
    std::vector<std::vector<individual>> pop_;
  };

  std::ostream &operator<<(std::ostream &, const population &);

  ///
  /// \param[in] c coordinates of an \a individual.
  /// \return a reference to the \a individual at coordinates \a c.
  ///
  inline
  individual &population::operator[](const coord &c)
  {
    assert(c.layer < layers());
    assert(c.index < individuals(c.layer));
    return pop_[c.layer][c.index];
  }

  ///
  /// \param[in] c coordinates of an individual.
  /// \return a constant reference to the individual at coordinates \a c.
  ///
  inline
  const individual &population::operator[](const coord &c) const
  {
    assert(c.layer < layers());
    assert(c.index < individuals(c.layer));
    return pop_[c.layer][c.index];
  }

  ///
  /// \return number of layers for the population.
  ///
  inline
  size_t population::layers() const
  {
    return pop_.size();
  }

  ///
  /// \param l a layer of the population.
  /// \return the number of individuals in the l-th layer of the population.
  ///
  inline
  size_t population::individuals(size_t l) const
  {
    assert(l < layers());
    return pop_[l].size();
  }

  ///
  /// \return a constant reference to the active environment.
  ///
  inline
  const environment &population::env() const
  {
    return pop_[0][0].env();
  }

  ///
  /// \example example2.cc
  /// Creates a random population and shows its content.
  ///
}  // namespace vita

#endif  // POPULATION_H
