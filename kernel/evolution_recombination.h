/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_RECOMBINATION_H)
#define      VITA_EVOLUTION_RECOMBINATION_H

#include "kernel/population.h"
#include "kernel/vitafwd.h"

namespace vita { namespace recombination {
  ///
  /// \brief The operation strategy (crossover, recombination, mutation...) for
  ///        the \a evolution class
  ///
  /// \tparam T type of individual
  ///
  /// A recombination acts upon sets of individuals to generate offspring
  /// (this definition generalizes the traditional mutation and crossover
  /// operators).
  ///
  /// Operator application is atomic from the point of view of the
  /// evolutionary algorithm and every recombination is applied to a well
  /// defined list of individuals, without dependencies upon past history.
  ///
  /// In the strategy design pattern, this class is the strategy interface and
  /// vita::evolution is the context.
  ///
  /// This is an abstract class: introduction of new operators or redefinition
  /// of existing ones is obtained implementing recombination::strategy.
  ///
  /// The design adheres to the NVI pattern ("Virtuality" in C/C++ Users
  /// Journal September 2001).
  ///
  /// \see
  /// * <http://en.wikipedia.org/wiki/Strategy_pattern>
  /// * <http://www.gotw.ca/publications/mill18.htm>
  ///
  template<class T>
  class strategy
  {
  public:
    strategy(const population<T> &, evaluator<T> &, summary<T> *const);
    virtual ~strategy() {}

    std::vector<T> run(const std::vector<coord> &);

  private:  // NVI template methods
    virtual std::vector<T> run_nvi(const std::vector<coord> &) = 0;

  protected:
    const population<T> &pop_;
    evaluator<T> &eva_;
    summary<T> *stats_;
  };

  ///
  /// This class defines the program skeleton of a standard genetic
  /// programming crossover plus mutation operation. It's a template method
  /// design pattern: one or more of the algorithm steps can be overriden
  /// by subclasses to allow differing behaviours while ensuring that the
  /// overarching algorithm is still followed.
  ///
  template<class T>
  class base : public strategy<T>
  {
  public:
    using base::strategy::strategy;

  private:  // NVI template methods
    virtual std::vector<T> run_nvi(const std::vector<coord> &) override;
  };

  ///
  /// This is based on the differential evolution four members crossover.
  ///
  template<class T>
  class de : public strategy<T>
  {
  public:
    using de::strategy::strategy;

  private:  // NVI template methods
    virtual std::vector<T> run_nvi(const std::vector<coord> &) override;
  };

#include "kernel/evolution_recombination_inl.h"
} }  // namespace vita::recombination

#endif  // Include guard
