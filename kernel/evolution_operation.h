/**
 *
 *  \file evolution_operation.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_OPERATION_H)
#define      EVOLUTION_OPERATION_H

#include <vector>

#include "individual.h"

namespace vita
{
  class evolution;
  class summary;

  ///
  /// \brief The operation strategy (crossover, recombination, mutation...) for
  ///        the \a evolution class.
  ///
  /// In the strategy design pattern, this class is the strategy interface and
  /// \a evolution is the context.
  ///
  /// An operation act upon sets of individuals to generate offspring (this
  /// definition generalizes the traditional mutation and crossover operators).
  /// This is an abstract class: introduction of new operators or redefinition
  /// of existing ones is obtained implementing \a operation_strategy.
  ///
  /// Operator application is atomic from the point of view of the
  /// evolutionary algorithm and every operation is applied to a well defined
  /// list of individuals, without dependencies upon past history.
  ///
  /// \see
  /// http://en.wikipedia.org/wiki/Strategy_pattern
  ///
  class operation_strategy
  {
  public:
    typedef std::shared_ptr<operation_strategy> ptr;

    operation_strategy(const evolution *const, summary *const);
    virtual ~operation_strategy() {}

    // Defining offspring as a set of individuals lets the generalized operation
    // encompass recent additions, such as scan mutation, that generates
    // numerous offspring from a single parent.
    virtual std::vector<individual> run(const std::vector<index_t> &) = 0;

  protected:
    const evolution *const evo_;
    summary *stats_;
  };

  ///
  /// This class defines the program skeleton of a standard genetic
  /// programming crossover plus mutation operation. It's a template method
  /// design pattern: one or more of the algorithm steps can be overriden
  /// by subclasses to allow differing behaviors while ensuring that the
  /// overarching algorithm is still followed.
  ///
  class standard_op : public operation_strategy
  {
  public:
    standard_op(const evolution *const, summary *const);

    virtual std::vector<individual> run(const std::vector<index_t> &);
  };
}  // namespace vita

#endif  // EVOLUTION_OPERATION_H
