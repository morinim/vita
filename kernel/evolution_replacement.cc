/**
 *
 *  \file evolution_replacement.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <boost/optional.hpp>

#include "evolution_replacement.h"
#include "environment.h"
#include "evolution.h"

namespace vita
{
  replacement_strategy::replacement_strategy(evolution *const evo)
    : evo_(evo)
  {
  }

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
  /// * elitism is \c true => child replaces a member of the population only if
  ///   child is better.
  ///
  void family_competition_rp::run(const std::vector<coord> &parent,
                                  const std::vector<individual> &offspring,
                                  summary *const s)
  {
    population &pop = evo_->population();
    assert(!boost::indeterminate(pop.env().elitism));

    const fitness_t fit_off(evo_->fitness(offspring[0]));

    const fitness_t f_parent[] =
    {
      evo_->fitness(pop[parent[0]]),
      evo_->fitness(pop[parent[1]])
    };
    const bool id_worst(f_parent[0] < f_parent[1] ? 0 : 1);

    if (pop.env().elitism)
    {
      if (fit_off > f_parent[id_worst])
        pop[parent[id_worst]] = offspring[0];
    }
    else  // !elitism
    {
      // THIS CODE IS APPROPRIATE ONLY WHEN FITNESS IS A SCALAR. It will work
      // when fitness is a vector but the replacement probability should be
      // calculated in a better way.

      //double replace(1.0 / (1.0 + exp(f_parent[id_worst][0] - fit_off[0])));
      double replace(1.0 - (fit_off[0] /
                            (fit_off[0] + f_parent[id_worst][0])));
      if (random::boolean(replace))
        pop[parent[id_worst]] = offspring[0];
      else
      {
        //replace = 1.0 / (1.0 + exp(f_parent[!id_worst][0] - fit_off[0]));
        replace = 1.0 - (fit_off[0] / (fit_off[0] + f_parent[!id_worst][0]));

        if (random::boolean(replace))
          pop[parent[!id_worst]] = offspring[0];
      }

      //pop[parent[id_worst]] = offspring[0];
    }

    if (fit_off > s->best->fitness)
    {
      summary::best_ b{offspring[0], fit_off};
      s->last_imp = s->gen;
      s->best     =      b;
    }
  }

  ///
  /// \param[in] evo pointer to the evolution object that is using the
  ///                kill_tournament.
  ///
  kill_tournament::kill_tournament(evolution *const evo)
    : replacement_strategy(evo)
  {
  }

  ///
  /// \param[in] pusher coordinates of a new individual.
  /// \param[in] pushed coordinates of a "settled" individual.
  /// \return \c true if \a pusher can replace \a pushed (according to ALPS
  ///         criterion).
  ///
  bool kill_tournament::can_replace(const coord &pusher,
                                    const coord &pushed) const
  {
    population &pop(evo_->population());

    return can_replace(pop[pusher], pushed);
  }
  bool kill_tournament::can_replace(const individual &pusher,
                                    const coord &pushed) const
  {
    population &pop(evo_->population());
    const auto max_age(pop.max_age(pushed.layer));

    return
      (pop[pushed].age > max_age && pusher.age <= max_age) ||
      evo_->fitness(pusher) > evo_->fitness(pop[pushed]);
  }

  ///
  /// \param[in] pusher coordinates of an individual that will eventually be
  ///                   moved to the upper layer.
  ///
  /// This is a key function for the ALPS algorithm.
  /// When an individual, A, is being moved up to a layer \f$L_i\f$, then what
  /// is done is a check to see if there is an individual in \f$L_i\f$ that has
  /// worse fitness than A or is too old to be in \f$L_i\f$.
  /// If it exits (let's call this individual B), we try to move B up to layer
  /// \f$L_i +  1\f$.
  ///
  void kill_tournament::try_move_up(const coord &pusher)
  {
    population &pop(evo_->population());
    const auto next_layer(pusher.layer + 1);
/*
    if (next_layer < pop.layers())
    {
      const auto individuals(pop.individuals(next_layer));
      const auto rounds(pop.env().tournament_size);

      const auto pusher_fit(evo_->fitness(pop[pusher]));

      boost::optional<coord> worst;
      unsigned i(0);
      do
      {
        const coord target{next_layer, random::between<size_t>(0, individuals)};
        const auto target_fit(evo_->fitness(pop[target]));

        if (can_replace(pusher, target))
        {
          if (!worst || evo_->fitness(pop[*worst]) > target_fit)
            worst = target;
        }
      } while (++i < rounds);

      if (worst)
      {
        try_move_up(*worst);
        pop[*worst] = pop[pusher];
      }
    }
*/

    if (next_layer < pop.layers())
    {
      const auto individuals(pop.individuals(next_layer));
      const auto rounds(pop.env().tournament_size);

      coord worst{next_layer, random::between<size_t>(0, individuals)};
      auto fit_worst(evo_->fitness(pop[worst]));
      for (unsigned i(1); i < rounds; ++i)
      {
        const coord candidate{next_layer,
                              random::between<size_t>(0, individuals)};

        const auto fit_candidate(evo_->fitness(pop[candidate]));
        if (fit_candidate < fit_worst)
        {
          worst = candidate;
          fit_worst = fit_candidate;
        }
      }

      if (can_replace(pusher, worst))
      {
        try_move_up(worst);
        pop[worst] = pop[pusher];
      }
    }
  }

  ///
  /// \param[in] parent indexes of the candidate parents.
  ///                   The list is sorted in descending fitness, so the
  ///                   last element is the index of the worst individual of
  ///                   the tournament.
  /// \param[in] offspring vector of the "children".
  /// \param[in] s statistical \a summary.
  ///
  /// Parameters from the environment:
  /// * elitism is \c true => child replaces a member of the population only if
  ///   child is better.
  ///
  void kill_tournament::run(const std::vector<coord> &parent,
                            const std::vector<individual> &offspring,
                            summary *const s)
  {
    population &pop(evo_->population());

    const fitness_t fit_off(evo_->fitness(offspring[0]));

    // In old versions of Vita, the individual to be replaced was chosen with
    // an ad-hoc kill tournament. Something like:
    //
    //   const coord rep_idx(kill_tournament(parent[0]));
    //
    // Now we perform just one tournament for choosing the parents; the
    // individual to be replaced is selected among the worst individuals of
    // this tournament.
    // The new way is simpler and more general. Note that when tournament_size
    // is greater than 2 we perform a traditional selection / replacement
    // scheme; if it is smaller we perform a family competition replacement
    // (aka deterministic / probabilistic crowding).
    const coord rep_idx(parent.back());
    const fitness_t f_rep_idx(evo_->fitness(pop[rep_idx]));
    const bool replace(f_rep_idx < fit_off);

    assert(!boost::indeterminate(pop.env().elitism));
    if (!pop.env().elitism || replace)
    {
      try_move_up(rep_idx);
      pop[rep_idx] = offspring[0];
    }

    if (fit_off > s->best->fitness)
    {
      s->last_imp =                  s->gen;
      s->best     = {offspring[0], fit_off};
    }
  }
}  // namespace vita
