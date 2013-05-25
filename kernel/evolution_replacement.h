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

#include "vita.h"

namespace vita
{
  template<class T> class evolution;

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
  template<class T>
  class replacement_strategy
  {
  public:
    typedef typename std::shared_ptr<replacement_strategy<T>> ptr;

    explicit replacement_strategy(evolution<T> *const);
    virtual ~replacement_strategy() {}

    virtual void run(const std::vector<size_t> &, const std::vector<T> &,
                     summary<T> *const) = 0;

  protected:
    evolution<T> *const evo_;
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
  template<class T>
  class family_competition_rp : public replacement_strategy<T>
  {
  public:
    explicit family_competition_rp(evolution<T> *const);

    virtual void run(const std::vector<size_t> &, const std::vector<T> &,
                     summary<T> *const) override;
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
  template<class T>
  class kill_tournament : public replacement_strategy<T>
  {
  public:
    explicit kill_tournament(evolution<T> *const);

    virtual void run(const std::vector<size_t> &, const std::vector<T> &,
                     summary<T> *const) override;
  };

  template<class T>
  class pareto_tournament : public replacement_strategy<T>
  {
  public:
    explicit pareto_tournament(evolution<T> *const);

    virtual void run(const std::vector<size_t> &, const std::vector<T> &,
                     summary<T> *const) override;
  };

#include "evolution_replacement_inl.h"
}  // namespace vita

#endif  // EVOLUTION_REPLACEMENT_H
