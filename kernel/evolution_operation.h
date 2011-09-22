/**
 *
 *  \file evolution_operation.h
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
  /// An operation act upon sets of individuals to generate offspring (this
  /// definition generalizes the traditional mutation and crossover operators).
  /// This is an abstract \c class, introduction of new operators or
  /// redefinition of existing ones is obtained implementing
  /// \a operation_strategy. Operator application is atomic from the point of
  /// view of the evolutionary algorithm and every operation is applied to a
  /// well defined list of individuals, without dependencies upon past history.
  ///
  class operation_strategy
  {
  public:
    operation_strategy(const evolution *const, summary *const);

    // Defining offspring as a set of individuals lets the generalized operation
    // encompass recent additions, such as scan mutation, that generates
    // numerous offspring from a single parent.
    virtual std::vector<individual> operator()(
      const std::vector<locus_t> &) = 0;

  protected:
    const evolution *const evo_;
    summary *stats_;
  };

  ///
  /// operation_factory \c class creates a new \a operation_strategy (the
  /// strategy for the \a evolution \c class (the context).
  ///
  class operation_factory
  {
  public:
    enum operation {crossover_mutation = 0};

    operation_factory(const evolution *const, summary *const);
    ~operation_factory();

    operation_strategy &operator[](unsigned) const;
    unsigned add(operation_strategy *const);

  private:
    std::vector<operation_strategy *> strategy_;
  };
}  // namespace vita

#endif  // EVOLUTION_OPERATION_H
