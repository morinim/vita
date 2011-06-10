/**
 *
 *  \file evolution_operation.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini. All rights reserved.
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

#if !defined(EVOLUTION_OPERATION_H)
#define      EVOLUTION_OPERATION_H

#include <vector>

#include "kernel/vita.h"
#include "kernel/individual.h"

namespace vita
{
  class evolution;
  class summary;

  ///
  /// The operation strategy (crossover, recombination, mutation...) for the
  /// \a evolution \c class. In the strategy design pattern, this \c class is
  /// the strategy interface and \a evolution is the context.
  ///
  class operation_strategy
  {
  public:
    explicit operation_strategy(const evolution *const);

    virtual std::vector<individual> run(const std::vector<unsigned> &,
                                        summary *const) = 0;

  protected:
    const evolution *const _evo;
  };

  ///
  /// operation_factory \c class creates a new strategy for the \a evolution
  /// \c class (the context).
  ///
  class operation_factory
  {
  public:
    enum operation {unicross_mutation = 0};

    explicit operation_factory(const evolution *const);
    ~operation_factory();

    operation_strategy *get(unsigned);
    unsigned put(operation_strategy *const);

  private:
    std::vector<operation_strategy *> _strategy;
  };
}  // namespace vita

#endif  // EVOLUTION_OPERATION_H
