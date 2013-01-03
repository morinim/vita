/**
 *
 *  \file evolution_selection.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_SELECTION_H)
#define      EVOLUTION_SELECTION_H

#include <vector>

#include "vita.h"

namespace vita
{
  class evolution;

  ///
  /// \brief The selection strategy (tournament, fitness proportional...) for
  ///        the \a evolution class.
  ///
  /// In the strategy design pattern, this class is the strategy interface and
  /// \a evolution is the context.
  ///
  /// \see
  /// http://en.wikipedia.org/wiki/Strategy_pattern
  ///
  class selection_strategy
  {
  public:
    typedef std::shared_ptr<selection_strategy> ptr;

    explicit selection_strategy(const evolution *const);
    virtual ~selection_strategy() {}

    virtual std::vector<index_t> run() = 0;

  protected:
    const evolution *const evo_;
  };

  ///
  /// Tournament selection is a method of selecting an individual from a
  /// population of individuals. It involves running several "tournaments"
  /// among a few individuals chosen "at random" from the population. The
  /// winner of each tournament (the one with the best fitness) is selected
  /// for crossover.
  ///
  /// Selection pressure is easily adjusted by changing the tournament size.
  /// If the tournament size is larger, weak individuals have a smaller chance
  /// to be selected.
  /// A 1-way tournament selection is equivalent to random selection.
  ///
  /// Tournament selection has several benefits: it is efficient to code, works
  /// on parallel architectures and allows the selection pressure to be easily
  /// adjusted.
  ///
  class tournament_selection : public selection_strategy
  {
  public:
    explicit tournament_selection(const evolution *const);

    virtual std::vector<index_t> run();
  };

  ///
  /// Very simple selection strategy: pick a set of random individuals.
  ///
  class random_selection : public selection_strategy
  {
  public:
    explicit random_selection(const evolution *const);

    virtual std::vector<index_t> run();
  };
}  // namespace vita

#endif  // EVOLUTION_SELECTION_H
