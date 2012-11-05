/**
 *
 *  \file evolution_operation.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
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

  ///
  /// This \c class defines the program skeleton of a standard genetic
  /// programming crossover plus mutation operation. It's a template method
  /// design pattern: one or more of the algorithm steps can be overriden
  /// by subclasses to allow differing behaviors while ensuring that the
  /// iverarching algorithm is still followed.
  ///
  class standard_op : public operation_strategy
  {
  public:
    standard_op(const evolution *const, summary *const);

    std::vector<individual> operator()(const std::vector<index_t> &);

    summary *stats;
  };

  standard_op::standard_op(const evolution *const evo, summary *const s)
    : operation_strategy(evo, s)
  {
  }

  ///
  /// \return the offspring.
  ///
  /// This is a quite standard crossover + mutation operator.
  ///
  std::vector<individual> standard_op::operator()(
    const std::vector<index_t> &parent)
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

  operation_factory::operation_factory(const evolution *const evo,
                                       summary *const stats)
  {
    assert(evo);
    assert(stats);

    unsigned VARIABLE_IS_NOT_USED i;

    i = add(new standard_op(evo, stats));

    assert(i - 1 == k_crossover_mutation);
  }

  operation_factory::~operation_factory()
  {
    // Only predefined operation strategies should be deleted. User defined
    // operation aren't under our responsability.
    delete strategy_[k_crossover_mutation];
  }

  operation_strategy &operation_factory::operator[](unsigned s) const
  {
    assert(s < strategy_.size());
    return *strategy_[s];
  }

  unsigned operation_factory::add(operation_strategy *const s)
  {
    assert(s);
    strategy_.push_back(s);
    return strategy_.size();
  }
}  // namespace vita
