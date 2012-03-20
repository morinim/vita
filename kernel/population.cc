/**
 *
 *  \file population.cc
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
