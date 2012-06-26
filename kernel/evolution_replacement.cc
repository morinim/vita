/**
 *
 *  \file evolution_replacement.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "kernel/evolution_replacement.h"
#include "kernel/environment.h"
#include "kernel/evolution.h"

namespace vita
{
  replacement_strategy::replacement_strategy(evolution *const evo)
    : evo_(evo)
  {
  }

  ///
  /// This is a family competition replacement scheme (see "Replacement
  /// Strategies to Preserve Useful Diversity in Steady-State Genetic
  /// Algorithms" - Lozano, Herrera, Cano - 2003).
  /// We assume that the parents would be ones of the members of the population
  /// closest to the new elements. In this way, children compete with their
  /// parents to be included in the population. A child replaces the worst
  /// parent if it has a higher fitness (deterministic crowding and
  /// elitist recombination); if \c elitism is \c false, the winner of the
  /// parent-offspring tournament is chosen by using a probability proportional
  /// to the fitness (probabistic crowding).
  ///
  class family_competition_rp : public replacement_strategy
  {
  public:
    explicit family_competition_rp(evolution *const);

    virtual void operator()(const std::vector<index_t> &,
                            const std::vector<individual> &,
                            summary *const);
  };

  family_competition_rp::family_competition_rp(evolution *const evo)
    : replacement_strategy(evo)
  {
  }

  ///
  /// \param[in] parent indexes of the parents (in the population).
  /// \param[in] offspring vector of the "children".
  /// \param[in] s statistical \a summary.
  ///
  /// Parameters from the environment:
  /// <ul>
  /// <li>
  ///   elitism is \c true => child replaces a member of the population only if
  ///   child is better.
  /// </li>
  /// </ul>
  ///
  void family_competition_rp::operator()(
    const std::vector<index_t> &parent,
    const std::vector<individual> &offspring,
    summary *const s)
  {
    assert(!boost::indeterminate(pop.env().elitism));

    population &pop = evo_->population();

    const score_t score_off(evo_->score(offspring[0]));

    const fitness_t f_parent[] =
    {
      evo_->fitness(pop[parent[0]]),
      evo_->fitness(pop[parent[1]])
    };
    const bool id_worst(f_parent[0] < f_parent[1] ? 0 : 1);

    if (pop.env().elitism)
    {
      if (score_off.fitness > f_parent[id_worst])
        pop[parent[id_worst]] = offspring[0];
    }
    else  // !elitism
    {
      //double replace(1.0/(1.0 + exp(f_parent[id_worst] - score_off.fitness)));
      double replace(1.0 - (score_off.fitness /
                            (score_off.fitness + f_parent[id_worst])));
      if (random::boolean(replace))
        pop[parent[id_worst]] = offspring[0];
      else
      {
        //replace = 1.0 / (1.0 + exp(f_parent[!id_worst] - score_off.fitness));
        replace = 1.0 - (score_off.fitness /
                         (score_off.fitness + f_parent[!id_worst]));

        if (random::boolean(replace))
          pop[parent[!id_worst]] = offspring[0];
      }

      //pop[parent[id_worst]] = offspring[0];
    }

    if (score_off.fitness > s->best->score.fitness)
    {
      s->last_imp =                    s->gen;
      s->best     = {offspring[0], score_off};
    }
  }

  class tournament_rp : public replacement_strategy
  {
  public:
    explicit tournament_rp(evolution *const);

    virtual void operator()(const std::vector<index_t> &,
                            const std::vector<individual> &,
                            summary *const);

  private:
    index_t tournament(index_t) const;
  };

  tournament_rp::tournament_rp(evolution *const evo)
    : replacement_strategy(evo)
  {
  }

  ///
  /// \param[in] target index of an \a individual in the \a population.
  /// \return index of the worst \a individual found.
  ///
  /// Tournament selection works by selecting a number of individuals from the
  /// population at random, a tournament, and then choosing only the worst
  /// of those individuals.
  /// Recall that better individuals have highter fitnesses.
  ///
  index_t tournament_rp::tournament(index_t target) const
  {
    const population &pop(evo_->population());

    const unsigned n(pop.size());
    const unsigned mate_zone(*pop.env().mate_zone);
    const unsigned rounds(*pop.env().rep_tournament);

    index_t sel(random::ring(target, mate_zone, n));
    for (unsigned i(1); i < rounds; ++i)
    {
      const index_t j(random::ring(target, mate_zone, n));

      const fitness_t fit_j(evo_->fitness(pop[j]));
      const fitness_t fit_sel(evo_->fitness(pop[sel]));
      if (fit_j < fit_sel)
        sel = j;
    }

    return sel;
  }

  ///
  /// \param[in] parent indexes of the parents (in the population).
  /// \param[in] offspring vector of the "children".
  /// \param[in] s statistical \a summary.
  ///
  /// Parameters from the environment:
  /// <ul>
  /// <li>
  ///   elitism is \c true => child replaces a member of the population only if
  ///   child is better.
  /// </li>
  /// </ul>
  ///
  void tournament_rp::operator()(const std::vector<index_t> &parent,
                                 const std::vector<individual> &offspring,
                                 summary *const s)
  {
    population &pop = evo_->population();

    const score_t score_off(evo_->score(offspring[0]));

    const unsigned rep_idx(tournament(parent[0]));
    const fitness_t f_rep_idx(evo_->fitness(pop[rep_idx]));
    const bool replace(f_rep_idx < score_off.fitness);

    assert(!boost::indeterminate(pop.env().elitism));
    if (!pop.env().elitism || replace)
      pop[rep_idx] = offspring[0];

    if (score_off.fitness > s->best->score.fitness)
    {
      s->last_imp =                    s->gen;
      s->best     = {offspring[0], score_off};
    }
  }

  replacement_factory::replacement_factory(evolution *const evo)
  {
    unsigned i;

    i = add(new family_competition_rp(evo));
    assert(i == k_crowding);

    i = add(new tournament_rp(evo));
    assert(i == k_tournament);
  }

  replacement_factory::~replacement_factory()
  {
    // Only predefined operation strategies should be deleted. User defined
    // operation aren't under our responsability.
    delete strategy_[k_crowding];
    delete strategy_[k_tournament];
  }

  replacement_strategy &replacement_factory::operator[](unsigned s)
  {
    assert(s < strategy_.size());
    return *strategy_[s];
  }

  unsigned replacement_factory::add(replacement_strategy *const s)
  {
    assert(s);
    strategy_.push_back(s);
    return strategy_.size();
  }
}  // namespace vita
