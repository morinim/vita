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
  /// Creates a random population (initial size \a e.individuals).
  ///
  population::population(const environment &e)
  {
    assert(e.debug(true, true));

    pop_.reserve(e.individuals);
    pop_.clear();

    for (size_t i(0); i < e.individuals; ++i)
      pop_.emplace_back(e, true);

    assert(debug(true));
  }

  ///
  /// Increments the age of each individual in the population.
  ///
  void population::inc_age()
  {
    for (individual &i : pop_)
      ++i.age;
  }

  ///
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool population::debug(bool verbose) const
  {
    for (const individual &i : pop_)
      if (!i.debug(verbose))
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
    size_t n_elem(0);
    if (!(in >> n_elem))
      return false;

    population p(env());
    for (size_t i(0); i < n_elem; ++i)
      if (!p[i].load(in))
        return false;

    *this = p;
    return true;
  }

  ///
  /// \param[out] out output stream.
  /// \return \c true if population was saved correctly.
  ///
  bool population::save(std::ostream &out) const
  {
    out << individuals() << std::endl;

    for (const individual &i : *this)
      i.save(out);

    return out.good();
  }

  ///
  /// \param[in,out] s output \c stream.
  /// \param[in] pop population to be listed.
  /// \return the output \c stream.
  ///
  std::ostream &operator<<(std::ostream &s, const population &pop)
  {
    for (const individual &i : pop)
      s << i << std::endl;

    return s;
  }
}  // namespace vita
