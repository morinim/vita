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
  /// \brief The replacement strategy (random, elitist...) for the \a evolution
  ///        class.
  ///
  /// In the strategy design pattern, this class is the strategy interface and
  /// \a evolution is the context.
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

    virtual void run(const std::vector<size_t> &,
                     const std::vector<individual> &, summary *const) = 0;

  protected:
    evolution *const evo_;
  };

  ///
  /// \brief A family competition replacement scheme.
  ///
  /// We assume that the parents would be ones of the members of the population
  /// closest to the new elements. In this way, children compete with their
  /// parents to be included in the population.
  ///
  /// A child replaces the worst parent if it has a higher fitness
  /// (_deterministic crowding_ and _elitist recombination_); if elitism is
  /// \c false, the winner of the parent-offspring tournament is chosen by
  /// using a probability proportional to the fitness (_probabistic crowding_).
  ///
  /// \see
  /// "Replacement Strategies to Preserve Useful Diversity in Steady-State
  /// Genetic Algorithms" - Lozano, Herrera, Cano - 2003.
  ///
  class family_competition_rp : public replacement_strategy
  {
  public:
    explicit family_competition_rp(evolution *const);

    virtual void run(const std::vector<size_t> &,
                     const std::vector<individual> &, summary *const) override;
  };

  ///
  /// \brief Tournament based replacement scheme.
  ///
  /// This strategy select an individual for replacement by kill tournament:
  /// pick a number of individuals at random and replace the worst.
  ///
  /// \see
  /// "Replacement Strategies in Steady State Genetic Algorithms: Static
  /// Environments" - Jim Smith, Frank Vavak.
  ///
  class kill_tournament : public replacement_strategy
  {
  public:
    explicit kill_tournament(evolution *const);

    virtual void run(const std::vector<size_t> &,
                     const std::vector<individual> &, summary *const) override;
  };
}  // namespace vita

#endif  // EVOLUTION_REPLACEMENT_H
