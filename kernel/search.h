/**
 *
 *  \file search.h
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

#if !defined(SEARCH_H)
#define      SEARCH_H

#include "kernel/vita.h"
#include "kernel/fitness.h"

namespace vita
{
  template <class T> class distribution;
  class evolution;
  class individual;
  class problem;
  class summary;

  ///
  /// This \c class drives the evolution.
  ///
  class search
  {
  public:
    explicit search(problem *const);

    void arl(const individual &, evolution &);

    const individual &run(bool = true, unsigned = 1, fitness_t = -5.0);

    bool check() const;

  private:
    void log(const summary &, const distribution<fitness_t> &,
             unsigned, unsigned, unsigned) const;

    problem *prob_;
  };
}  // namespace vita

#endif  // SEARCH_H
