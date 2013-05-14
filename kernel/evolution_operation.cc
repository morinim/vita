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
  /// \param[in] parent a vector of ordered parents.
  /// \return the offspring.
  ///
  /// This is a quite standard crossover + mutation operator.
  ///
  std::vector<individual> standard_op::run(const std::vector<size_t> &parent)
  {
    assert(parent.size() >= 2);

    const population &pop(evo_->population());
    const environment &env(pop.env());

    assert(env.p_cross);
    assert(env.p_mutation);
    assert(env.brood_recombination);

    const auto r1(parent[0]), r2(parent[1]);

    if (random::boolean(*env.p_cross))
    {
      individual off(individual::crossover(pop[r1], pop[r2]));
      ++stats_->crossovers;

      // This could be an original contribution of Vita... but it's hard
      // to be sure.
      // It remembers of the hereditary repulsion constraint (I guess you could
      // call it signature repulsion) and seems to:
      // * maintain diversity during the exploration phase;
      // * optimize the exploitation phase.
      while (pop[r1].signature() == off.signature() ||
             pop[r2].signature() == off.signature())
        stats_->mutations += off.mutation();

      //if (evo_->seen(off))
      //  stats_->mutations += off.mutation();

      if (*env.brood_recombination > 0)
      {
        fitness_t fit_off(evo_->fast_fitness(off));

        unsigned i(0);
        do
        {
          individual tmp(individual::crossover(pop[r1], pop[r2]));

          while (pop[r1].signature() == tmp.signature() ||
                 pop[r2].signature() == tmp.signature())
            stats_->mutations += tmp.mutation();

          const fitness_t fit_tmp(evo_->fast_fitness(tmp));
          if (fit_tmp > fit_off)
          {
            off     =     tmp;
            fit_off = fit_tmp;
          }
        } while (++i < *env.brood_recombination);

        stats_->crossovers += i;
      }

      assert(off.debug());
      return {off};
    }
    else // !crossover
    {
      individual off(pop[random::boolean() ? r1 : r2]);
      stats_->mutations += off.mutation();

      assert(off.debug());
      return {off};
    }
  }
}  // namespace vita
