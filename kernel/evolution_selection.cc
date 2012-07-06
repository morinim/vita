/**
 *
 *  \file evolution_selection.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
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

    virtual std::vector<index_t> operator()();
  };

  tournament_selection::tournament_selection(const evolution *const evo)
    : selection_strategy(evo)
  {
  }

  /*
  ///
  /// \param[in] target index of an \a individual in the \a population.
  /// \return index of the best \a individual found.
  ///
  /// Tournament selection works by selecting a number of individuals from the
  /// population at random, a tournament, and then choosing only the best
  /// of those individuals.
  /// Recall that better individuals have highter fitnesses.
  ///
  index_t tournament_selection::tournament(index_t target) const
  {
    const population &pop(evo_->population());

    const unsigned n(pop.size());
    const unsigned mate_zone(*pop.env().mate_zone);
    const unsigned rounds(*pop.env().tournament_size);

    index_t sel(random::ring(target, mate_zone, n));
    for (unsigned i(1); i < rounds; ++i)
    {
      const index_t j(random::ring(target, mate_zone, n));

      const fitness_t fit_j(evo_->fitness(pop[j]));
      const fitness_t fit_sel(evo_->fitness(pop[sel]));
      if (fit_j > fit_sel)
        sel = j;
    }

    return sel;
  }
  */

  ///
  /// \return a vector of indexes to individuals ordered in descending
  ///         fitness score.
  ///
  std::vector<index_t> tournament_selection::operator()()
  {
    const population &pop(evo_->population());

    const unsigned n(pop.size());
    const unsigned mate_zone(*pop.env().mate_zone);
    const unsigned rounds(*pop.env().tournament_size);
    const index_t target(random::between<unsigned>(0, n));

    assert(rounds);
    std::vector<index_t> ret(rounds);

    // This is the inner loop of an insertion sort algorithm. It is simple,
    // fast (if rounds is small) and doesn't perform too much comparisons.
    // DO NOT USE std::sort it is way slower.
    for (unsigned i(0); i < rounds; ++i)
    {
      const index_t new_index(random::ring(target, mate_zone, n));
      const fitness_t new_fitness(evo_->fitness(pop[new_index]));

      index_t j(0);
      while (j < i && new_fitness < evo_->fitness(pop[ret[j]]))
        ++j;

      for (index_t k(j); k < i; ++k)
        ret[k + 1] = ret[k];

      ret[j] = new_index;
    }

#if !defined(NDEBUG)
    for (unsigned i(0); i + 1 < rounds; ++i)
      assert(evo_->fitness(pop[ret[i]]) >= evo_->fitness(pop[ret[i + 1]]));
#endif

    return ret;
  }

  ///
  /// \param[in] evo pointer to the evolution class.
  ///
  selection_factory::selection_factory(const evolution *const evo)
  {
    unsigned i;

    i = add(new tournament_selection(evo));
    assert(i - 1 == k_tournament);
  }

  selection_factory::~selection_factory()
  {
    // Only predefined operation strategies should be deleted. User defined
    // operation aren't under our responsability.
    delete strategy_[k_tournament];
  }

  selection_strategy &selection_factory::operator[](unsigned s)
  {
    assert(s < strategy_.size());
    return *strategy_[s];
  }

  ///
  /// \param[in] s pointer to a selection_strategy.
  /// \return number of strategies inserted after (\a s included).
  ///
  unsigned selection_factory::add(selection_strategy *const s)
  {
    assert(s);
    strategy_.push_back(s);
    return strategy_.size();
  }
}  // namespace vita
