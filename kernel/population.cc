/**
 *
 *  \file population.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <ctime>
#include <fstream>

#include "kernel/population.h"
#include "kernel/random.h"

namespace vita
{
  ///
  /// \param[in] e base vita::environment.
  ///
  /// Creates a random population.
  ///
  population::population(const environment &e)
  {
    assert(e.check(true, true));

    pop_.reserve(*e.individuals);
    pop_.clear();

    for (unsigned i(0); i < *e.individuals; ++i)
      pop_.push_back(individual(e, true));

    assert(check());
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool population::check() const
  {
    for (unsigned i(0); i < size(); ++i)
      if (!pop_[i].check())
        return false;

    return true;
  }

  ///
  /// \param[in,out] s output \c stream.
  /// \param[in] pop population to be listed.
  /// \return the output \c stream.
  ///
  std::ostream &operator<<(std::ostream &s, const population &pop)
  {
    for (unsigned i(0); i < pop.size(); ++i)
      s << pop[i] << std::endl;

    return s;
  }
}  // namespace vita
