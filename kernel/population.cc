/**
 *
 *  \file population.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <fstream>

#include "population.h"

namespace vita
{
  ///
  /// \param[in] e base vita::environment.
  ///
  /// Creates a random population.
  ///
  population::population(const environment &e) : pop_(e.layers)
  {
    assert(e.debug(true, true));

    for (size_t l(0); l < e.layers; ++l)
      clear(e, l);

    assert(debug());
  }

  ///
  /// Resets layer 0 of the population.
  ///
  void population::reset_layer()
  {
    assert(layers());  // we reset a layer only when ALPS is enabled

    clear(env(), 0);
  }

  ///
  /// \param[in] e the environment used to build individuals.
  /// \param[in] l a layer of the population.
  ///
  /// Resets layer \a l of the population.
  ///
  void population::clear(const environment &e, size_t l)
  {
    assert(l < layers());

    const size_t ind_per_layer(e.individuals / e.layers);

    pop_[l].reserve(ind_per_layer);
    pop_[l].clear();

    for (size_t i(0); i < ind_per_layer; ++i)
      pop_[l].emplace_back(e, true);
  }

  ///
  /// \return the number of individuals in the population.
  ///
  size_t population::individuals() const
  {
    size_t n(0);

    for (size_t l(0); l < layers(); ++l)
      n += individuals(l);

    return n;
  }

  ///
  /// Increments the age of each individual in the population.
  ///
  void population::inc_age()
  {
    for (size_t l(0); l < layers(); ++l)
      for (individual &i : pop_[l])
        ++i.age;
  }

  ///
  /// \param[in] l the id of a layer.
  /// \return max age for an individual in layer \a l.
  ///
  /// Each age-layer in the population has a maximum age limit for individuals
  /// in it (except for the last layer wich can have individuals of any age).
  ///
  /// Parameters from the environment:
  /// * \a age_gap - a multiplier for the age limit of a layer;

  ///
  unsigned population::max_age(size_t l) const
  {
    const auto age_gap(env().age_gap);
    assert(age_gap);

    // Last layer can contain individuals of any age.
    if (l + 1 == layers())
      return std::numeric_limits<unsigned>::max();

    // A polynomial aging scheme.
    switch (l)
    {
    case 0:
      return age_gap;
    case 1:
      return age_gap + age_gap;
    default:
      return l * l * age_gap;
    }

    /*
      // Linear aging scheme.
      return age_gap * (l + 1);
    */
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool population::debug() const
  {
    for (size_t l(0); l < layers(); ++l)
      for (const individual &i : pop_[l])
        if (!i.debug())
          return false;

    return true;
  }

  ///
  /// \param[in] in input stream.
  /// \return \c true if population was loaded correctly.
  ///
  /// \note
  /// If the load operation isn't successful the current population isn't
  /// changed.
  ///
  bool population::load(std::istream &in)
  {
    population p(env());

    size_t n_layers(0);
    if (!(in >> n_layers))
      return false;

    p.pop_.resize(n_layers);

    for (size_t l(0); l < n_layers; ++l)
    {
      size_t n_elem(0);
      if (!(in >> n_elem))
        return false;

      for (size_t i(0); i < n_elem; ++i)
        if (!p[{l,i}].load(in))
          return false;
    }

    *this = p;
    return true;
  }

  ///
  /// \param[out] out output stream.
  /// \return \c true if population was saved correctly.
  ///
  bool population::save(std::ostream &out) const
  {
    out << layers() << std::endl;

    for (size_t l(0); l < layers(); ++l)
    {
      out << individuals(l) << std::endl;

      for (const individual &i : pop_[l])
        i.save(out);
    }

    return out.good();
  }

  ///
  /// \param[in,out] s output \c stream.
  /// \param[in] pop population to be listed.
  /// \return the output \c stream.
  ///
  std::ostream &operator<<(std::ostream &s, const population &pop)
  {
    for (size_t l(0); l < pop.layers(); ++l)
    {
      s << "LAYER " << l << std::endl << std::string('-',60) << std::endl;
      for (size_t i(0); i < pop.individuals(l); ++i)
        s << pop[{l, i}] << std::endl;
    }

    return s;
  }
}  // namespace vita
