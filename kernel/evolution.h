/**
 *
 *  \file evolution.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_H)
#define      EVOLUTION_H

#include <memory>

#include "analyzer.h"
#include "evaluator_proxy.h"
#include "evolution_operation.h"
#include "evolution_replacement.h"
#include "evolution_selection.h"
#include "population.h"

namespace vita
{
  class environment;
  class evolution;
  class individual;

  class summary
  {
  public:  // Constructor and support functions.
    summary();

    void clear();

  public:  // Data members.
    analyzer az;

    struct best_
    {
      individual    ind;
      fitness_t fitness;
    };

    boost::optional<best_> best;

    double speed;

    std::uintmax_t  mutations;
    std::uintmax_t crossovers;

    unsigned gen;
    unsigned last_imp;
  };

  ///
  /// Progressively evolves a population of programs over a series of
  /// generations.
  ///
  /// The evolutionary search uses the Darwinian principle of natural selection
  /// (survival of the fittest) and analogs of various naturally occurring
  /// operations, including crossover (sexual recombination), mutation...
  ///
  class evolution
  {
  public:
    evolution(const environment &, evaluator *,
              std::function<bool (const summary &)> = nullptr,
              std::function<void (unsigned)> = nullptr);

    const summary &run(unsigned);

    const vita::population &population() const;
    vita::population &population();

    fitness_t fitness(const individual &) const;
    fitness_t fast_fitness(const individual &) const;

    bool debug() const;

    selection_strategy::ptr     selection;
    operation_strategy::ptr     operation;
    replacement_strategy::ptr replacement;

  private:
    const environment &env() const { return pop_.env(); }
    double get_speed(double) const;
    analyzer get_stats() const;
    void log(unsigned) const;

    vita::population pop_;
    evaluator       *eva_;
    summary        stats_;

    std::function<bool (const summary &)> stop_condition_;
    std::function<void (unsigned)> shake_data_;
  };

  ///
  /// \return access to the population being evolved.
  ///
  inline
  vita::population &evolution::population()
  {
    return pop_;
  }

  ///
  /// \return constant reference to the population being evolved.
  ///
  inline
  const vita::population &evolution::population() const
  {
    return pop_;
  }

  ///
  /// \example example6.cc
  /// Simple symbolic regression example (\f$x^2+y^2-z^2\f$).
  ///

}  // namespace vita

#endif  // EVOLUTION_H
