/**
 *
 *  \file evolution.h
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

#if !defined(EVOLUTION_H)
#define      EVOLUTION_H

#include <memory>

#include "kernel/vita.h"
#include "kernel/analyzer.h"
#include "kernel/evaluator_proxy.h"
#include "kernel/evolution_operation.h"
#include "kernel/evolution_replacement.h"
#include "kernel/evolution_selection.h"
#include "kernel/population.h"

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
              std::function<void (unsigned)> = 0);

    const summary &operator()(bool, unsigned, unsigned = 0, unsigned = 0,
                              unsigned = 0);

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

    bool stop_condition() const;

    vita::population           pop_;
    std::shared_ptr<evaluator> eva_;
    summary                  stats_;

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
