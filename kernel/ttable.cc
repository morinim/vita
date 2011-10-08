/**
 *
 *  \file ttable.cc
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

#include "kernel/ttable.h"
#include "kernel/individual.h"

namespace vita
{
  ///
  /// \param[in] bits 2^\a bits is the number of elements of the table.
  ///
  /// Creates a new transposition (hash) table.
  ///
  ttable::ttable(unsigned bits)
    : k_mask((1 << bits)-1), table_(new slot[1 << bits]), probes_(0), hits_(0)
  {
    assert(check());
  }

  ///
  ttable::~ttable()
  {
    delete [] table_;
  }

  ///
  /// Clears the content and the statistical informations of the table
  /// (allocated size isn't changed).
  ///
  void ttable::clear()
  {
    probes_ = 0;
    hits_   = 0;

    for (unsigned i(0); i <= k_mask; ++i)
    {
      table_[i].hash = hash_t();
      table_[i].fit  = 0;
    }
  }

  ///
  /// \param[in] ind the individual to look for.
  /// \param[out] fit the fitness of the individual (if found).
  /// \return \c true if \a ind is found in the transposition table, \c false
  ///         otherwise.
  ///
  /// Looks for the fitness of an individual in the transposition table.
  ///
  bool ttable::find(const individual &ind, fitness_t *const fit) const
  {
    ++probes_;

    const hash_t h(ind.signature());

    const slot &s(table_[h.p1 & k_mask]);

    const bool ret(h == s.hash);

    if (ret)
    {
      ++hits_;
      *fit = s.fit;
    }

    return ret;
  }

  ///
  /// \param[in] ind a (possibly) new individual to be stored in the table.
  /// \param[out] fit the fitness of the individual.
  ///
  /// Stores fitness information in the transposition table.
  ///
  void ttable::insert(const individual &ind, fitness_t fit)
  {
    slot s;
    s.hash = ind.signature();
    s.fit  =             fit;

    table_[s.hash.p1 & k_mask] = s;
  }

  ///
  /// \return \c true if the table passes the internal consistency check.
  ///
  bool ttable::check() const
  {
    return probes() >= hits();
  }
}  // namespace vita
