/**
 *
 *  \file evolution_replacement.cc
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

#include "kernel/evolution_replacement.h"
#include "kernel/environment.h"
#include "kernel/evolution.h"

namespace vita
{
  replacement_strategy::replacement_strategy(evolution *const evo)
    : evo_(evo)
  {
  }

  class tournament_rp : public replacement_strategy
  {
  public:
    explicit tournament_rp(evolution *const);

    virtual void operator()(const std::vector<index_t> &,
                            const std::vector<individual> &,
                            summary *const);

  protected:
    index_t tournament(index_t) const;
  };

  tournament_rp::tournament_rp(evolution *const evo)
    : replacement_strategy(evo)
  {
  }

  index_t tournament_rp::tournament(index_t target) const
  {
    const population &pop(evo_->population());

    const unsigned n(pop.size());
    const unsigned mate_zone(pop.env().mate_zone);
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

    if (score_off.fitness - s->best->score.fitness > float_epsilon)
    {
      s->last_imp =                    s->gen;
      s->best     = {offspring[0], score_off};
    }
  }

  replacement_factory::replacement_factory(evolution *const evo)
  {
    add(new tournament_rp(evo));
  }

  replacement_factory::~replacement_factory()
  {
    // Only predefined operation strategies should be deleted. User defined
    // operation aren't under our responsability.
    delete strategy_[tournament];
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
