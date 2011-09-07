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

  struct summary
  {
    summary() { clear(); }

    void clear();

    boost::uint64_t ttable_probes;
    boost::uint64_t   ttable_hits;

    boost::uint64_t  mutations;
    boost::uint64_t crossovers;

    unsigned gen;
    unsigned testset;
    unsigned last_imp;

    analyzer az;

    individual  best;
    fitness_t f_best;
    double   sr_best;
  };

  class evolution
  {
  public:
    evolution(environment *const, evaluator *const);
    ~evolution();

    const summary &operator()(bool, unsigned, unsigned = 0, unsigned = 0,
                              unsigned = 0);

    const vita::population &population() const;
    vita::population &population();

    fitness_t fitness(const individual &) const;
    double success_rate(const individual &) const;

    void pick_stats(analyzer *const);

    bool check() const;

    selection_factory     selection;
    operation_factory     operation;
    replacement_factory replacement;

  private:
    void get_probes(boost::uint64_t *, boost::uint64_t *) const;
    void log(unsigned) const;
    void pick_stats();

    bool stop_condition() const;

    vita::population pop_;
    evaluator *const eva_;
    summary        stats_;
  };

  ///
  /// \example example6.cc
  /// Simple symbolic regression example (\f$x^2+y^2-z^2\f$).
  ///

}  // namespace vita

#endif  // EVOLUTION_H
