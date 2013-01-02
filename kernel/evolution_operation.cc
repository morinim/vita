/**
 *
 *  \file evolution_operation.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "evolution_operation.h"
#include "environment.h"
#include "evolution.h"

namespace vita
{
  operation_strategy::operation_strategy(const evolution *const evo,
                                         summary *const stats)
    : evo_(evo), stats_(stats)
  {
    assert(evo);
    assert(stats);
  }

  standard_op::standard_op(const evolution *const evo, summary *const s)
    : operation_strategy(evo, s)
  {
  }

  ///
  /// \return the offspring.
  ///
  /// This is a quite standard crossover + mutation operator.
  ///
  std::vector<individual> standard_op::run(const std::vector<index_t> &parent)
  {
    assert(parent.size() >= 2);

    const population &pop(evo_->population());
    const environment &env(pop.env());
    const unsigned r1(parent[0]), r2(parent[1]);

    if (random::boolean(*env.p_cross))
    {
      individual off(pop[r1].crossover(pop[r2]));
      ++stats_->crossovers;

      stats_->mutations += off.mutation();
/*
      do
      {
        stats_->mutations += off.mutation();
      } while (pop[r1].signature() == off.signature() ||
               pop[r2].signature() == off.signature());
*/
      if (*env.brood_recombination > 0)
      {
        fitness_t fit_off(evo_->fast_fitness(off));

        unsigned i(0);
        do
        {
          individual tmp(pop[r1].crossover(pop[r2]));
          stats_->mutations += tmp.mutation();

          ++stats_->crossovers;

          const fitness_t fit_tmp(evo_->fast_fitness(tmp));
          if (fit_tmp > fit_off)
          {
            off     =     tmp;
            fit_off = fit_tmp;
          }
        } while (++i < *env.brood_recombination);
      }

      assert(off.check());
      return {off};
    }
    else // !crossover
    {
      individual off(pop[random::boolean() ? r1 : r2]);
      stats_->mutations += off.mutation();

      assert(off.check());
      return {off};
    }
  }
}  // namespace vita
