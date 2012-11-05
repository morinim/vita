/**
 *
 *  \file evolution.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
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

  struct summary
  {
    summary();

    void clear();

    boost::uint64_t ttable_probes;
    boost::uint64_t   ttable_hits;

    boost::uint64_t  mutations;
    boost::uint64_t crossovers;

    double speed;

    unsigned gen;
    unsigned testset;
    unsigned last_imp;

    analyzer az;

    struct best_
    {
      individual ind;
      score_t  score;
    };
    boost::optional<best_> best;
  };

  class evolution
  {
  public:
    evolution(const environment &, evaluator *const,
              std::function<bool (const summary &)> = 0,
              std::function<void (unsigned)> = 0);

    const summary &operator()(
      bool, unsigned,
      selection_factory::strategy = selection_factory::k_tournament,
      operation_factory::strategy = operation_factory::k_crossover_mutation,
      replacement_factory::strategy = replacement_factory::k_tournament);

    const vita::population &population() const;
    vita::population &population();

    score_t score(const individual &) const;
    fitness_t fitness(const individual &) const;
    fitness_t fast_fitness(const individual &) const;

    bool check() const;

    selection_factory     selection;
    operation_factory     operation;
    replacement_factory replacement;

  private:
    double get_speed(double) const;
    void get_probes(boost::uint64_t *, boost::uint64_t *) const;
    analyzer get_stats() const;
    void log(unsigned) const;

    vita::population           pop_;
    std::shared_ptr<evaluator> eva_;
    summary                  stats_;

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
