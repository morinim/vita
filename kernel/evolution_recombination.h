/**
 *
 *  \file evolution_recombination.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_RECOMBINATION_H)
#define      EVOLUTION_RECOMBINATION_H

#include <vector>

#include "kernel/population.h"

namespace vita {
  template<class T> class evolution;
  template<class T> class summary;

namespace recombination {
  ///
  /// \brief The operation strategy (crossover, recombination, mutation...) for
  ///        the \a evolution class.
  ///
  /// In the strategy design pattern, this class is the strategy interface and
  /// \a evolution is the context.
  ///
  /// A recombination act upon sets of individuals to generate offspring (this
  /// definition generalizes the traditional mutation and crossover operators).
  /// This is an abstract class: introduction of new operators or redefinition
  /// of existing ones is obtained implementing \a recombination::strategy.
  ///
  /// Operator application is atomic from the point of view of the
  /// evolutionary algorithm and every recombination is applied to a well
  /// defined list of individuals, without dependencies upon past history.
  ///
  /// \see
  /// http://en.wikipedia.org/wiki/Strategy_pattern
  ///
  template<class T>
  class strategy
  {
  public:
    strategy(const evolution<T> *const, summary<T> *const);
    virtual ~strategy() {}

    // Defining offspring as a set of individuals lets the generalized
    // recombination encompass recent additions, such as scan mutation, that
    // generates numerous offspring from a single parent.
    virtual std::vector<T> run(const std::vector<coord> &) = 0;

  protected:
    const evolution<T> *const evo_;
    summary<T> *stats_;
  };

  ///
  /// This class defines the program skeleton of a standard genetic
  /// programming crossover plus mutation operation. It's a template method
  /// design pattern: one or more of the algorithm steps can be overriden
  /// by subclasses to allow differing behaviors while ensuring that the
  /// overarching algorithm is still followed.
  ///
  template<class T>
  class base : public strategy<T>
  {
  public:
    base(const evolution<T> *const, summary<T> *const);

    virtual std::vector<T> run(const std::vector<coord> &) override;
  };

#include "kernel/evolution_recombination_inl.h"
} }  // namespace vita::recombination

#endif  // EVOLUTION_RECOMBINATION_H
