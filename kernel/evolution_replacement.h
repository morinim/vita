/**
 *
 *  \file evolution_replacement.h
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

#if !defined(EVOLUTION_REPLACEMENT_H)
#define      EVOLUTION_REPLACEMENT_H

#include <vector>

#include "kernel/vita.h"
#include "kernel/individual.h"

namespace vita
{
  class evolution;
  class summary;

  ///
  /// The replacement strategy (random, elitist...) for the \a evolution
  /// \c class. In the strategy design pattern, this \c class is the strategy
  /// interface and \a evolution is the context.
  ///
  class replacement_strategy
  {
  public:
    explicit replacement_strategy(evolution *const);

    virtual void operator()(const std::vector<unsigned> &,
                            const std::vector<individual> &,
                            summary *const) = 0;

  protected:
    evolution *const evo_;
  };

  ///
  /// replacement_factory \c class creates a new \a replacement_strategy (the
  /// strategy) for the \a evolution \c class (the context).
  ///
  class replacement_factory
  {
  public:
    enum replacement {tournament = 0};

    explicit replacement_factory(evolution *const);
    ~replacement_factory();

    replacement_strategy &operator[](unsigned);
    unsigned put(replacement_strategy *const);

  private:
    std::vector<replacement_strategy *> strategy_;
  };
}  // namespace vita

#endif  // EVOLUTION_REPLACEMENT_H
