/**
 *
 *  \file evolution_selection.h
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

#if !defined(EVOLUTION_SELECTION_H)
#define      EVOLUTION_SELECTION_H

#include <vector>

#include "kernel/vita.h"

namespace vita
{
  class evolution;

  ///
  /// The selection strategy (tournament, fitness proportional...) for the
  /// \a evolution \c class. In the strategy design pattern, this \c class is
  /// the strategy interface and \a evolution is the context.
  ///
  class selection_strategy
  {
  public:
    explicit selection_strategy(const evolution *const);

    virtual std::vector<unsigned> operator()() = 0;

  protected:
    const evolution *const evo_;
  };

  ///
  /// selection_factory \c class creates a new \a selection_strategy (the
  /// strategy) for the \a evolution \c class (the context).
  ///
  class selection_factory
  {
  public:
    enum strategy {tournament = 0};

    explicit selection_factory(const evolution *const);
    ~selection_factory();

    selection_strategy &operator[](unsigned);
    unsigned put(selection_strategy *const);

  private:
    std::vector<selection_strategy *> _strategy;
  };
}  // namespace vita

#endif  // EVOLUTION_SELECTION_H
