/**
 *
 *  \file evolution_replacement.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_REPLACEMENT_H)
#define      EVOLUTION_REPLACEMENT_H

#include <vector>

#include "individual.h"

namespace vita
{
  class evolution;
  class summary;

  ///
  /// The replacement strategy (random, elitist...) for the \a evolution
  /// \c class. In the strategy design pattern, this \c class is the strategy
  /// interface and \a evolution is the context.
  ///
  /// \see
  /// http://en.wikipedia.org/wiki/Strategy_pattern
  ///
  class replacement_strategy
  {
  public:
    typedef std::shared_ptr<replacement_strategy> ptr;

    explicit replacement_strategy(evolution *const);
    virtual ~replacement_strategy() {}

    virtual void run(const std::vector<index_t> &,
                     const std::vector<individual> &,
                     summary *const) = 0;

  protected:
    evolution *const evo_;
  };

  ///
  /// This is a family competition replacement scheme.
  /// We assume that the parents would be ones of the members of the population
  /// closest to the new elements. In this way, children compete with their
  /// parents to be included in the population. A child replaces the worst
  /// parent if it has a higher fitness (deterministic crowding and
  /// elitist recombination); if \c elitism is \c false, the winner of the
  /// parent-offspring tournament is chosen by using a probability proportional
  /// to the fitness (probabistic crowding).
  ///
  /// \see
  /// "Replacement Strategies to Preserve Useful Diversity in Steady-State
  /// Genetic Algorithms" - Lozano, Herrera, Cano - 2003.
  ///
  class family_competition_rp : public replacement_strategy
  {
  public:
    explicit family_competition_rp(evolution *const);

    virtual void run(const std::vector<index_t> &,
                     const std::vector<individual> &,
                     summary *const);
  };

  ///
  /// This strategy select an individual for replacement by kill tournament:
  /// pick a number of parents at random and replace the worst.
  ///
  /// \see
  /// "Replacement Strategies in Steady State Genetic Algorithms: Static
  /// Environments" - Jim Smith, Frank Vavak.
  ///
  class kill_tournament : public replacement_strategy
  {
  public:
    explicit kill_tournament(evolution *const);

    virtual void run(const std::vector<index_t> &,
                     const std::vector<individual> &,
                     summary *const);
  };
}  // namespace vita

#endif  // EVOLUTION_REPLACEMENT_H
