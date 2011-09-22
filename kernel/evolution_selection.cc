/**
 *
 *  \file evolution_selection.cc
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

#include "kernel/evolution_selection.h"
#include "kernel/environment.h"
#include "kernel/evolution.h"

namespace vita
{
  selection_strategy::selection_strategy(const evolution *const evo)
    : evo_(evo)
  {
  }

  class tournament_selection : public selection_strategy
  {
  public:
    explicit tournament_selection(const evolution *const);

    virtual std::vector<locus_t> operator()();

  protected:
    unsigned tournament(unsigned) const;
  };

  tournament_selection::tournament_selection(const evolution *const evo)
    : selection_strategy(evo)
  {
  }

  ///
  /// \param[in] target index of an \a individual in the \a population.
  /// \return index of the best \a individual found.
  ///
  /// Tournament selection works by selecting a number of individuals from the
  /// population at random, a tournament, and then choosing only the best
  /// of those individuals.
  /// Recall that better individuals have highter fitnesses.
  ///
  unsigned tournament_selection::tournament(unsigned target) const
  {
    const unsigned n(evo_->population().size());
    const unsigned mate_zone(evo_->population().env().mate_zone);
    const unsigned rounds(evo_->population().env().par_tournament);

    unsigned sel(random::ring(target, mate_zone, n));
    for (unsigned i(1); i < rounds; ++i)
    {
      const unsigned j(random::ring(target, mate_zone, n));

      const fitness_t fit_j(evo_->fitness(evo_->population()[j]));
      const fitness_t fit_sel(evo_->fitness(evo_->population()[sel]));

      if (fit_j > fit_sel)
        sel = j;
    }

    return sel;
  }

  ///
  /// \return
  ///
  std::vector<locus_t> tournament_selection::operator()()
  {
    std::vector<locus_t> ret(2);

    ret[0] = tournament(evo_->population().size());
    ret[1] = tournament(ret[0]);

    return ret;
  }

  selection_factory::selection_factory(const evolution *const evo)
  {
    add(new tournament_selection(evo));
  }

  selection_factory::~selection_factory()
  {
    // Only predefined operation strategies should be deleted. User defined
    // operation aren't under our responsability.
    delete strategy_[tournament];
  }

  selection_strategy &selection_factory::operator[](unsigned s)
  {
    assert(s < strategy_.size());
    return *strategy_[s];
  }

  unsigned selection_factory::add(selection_strategy *const s)
  {
    assert(s);
    strategy_.push_back(s);
    return strategy_.size();
  }
}  // namespace vita
