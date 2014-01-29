/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(EVOLUTION_REPLACEMENT_H)
#define      EVOLUTION_REPLACEMENT_H

#include <vector>

#include "kernel/vita.h"

namespace vita {  namespace replacement {

  ///
  /// \brief The replacement strategy (random, tournament...) for the
  ///        vita::evolution_strategy class
  ///
  /// \tparam T type of individual.
  ///
  /// In the strategy design pattern, this class is the strategy interface and
  /// vita::evolution is the context.
  ///
  /// \see
  /// http://en.wikipedia.org/wiki/Strategy_pattern
  ///
  template<class T>
  class strategy
  {
  public:
    strategy(population<T> &, evaluator<T> &);
    virtual ~strategy() {}

    virtual void run(const std::vector<coord> &, const std::vector<T> &,
                     summary<T> *const) = 0;

  protected:
    population<T> &pop_;
    evaluator<T>  &eva_;
  };

  ///
  /// \brief A family competition replacement scheme
  ///
  /// \tparam T type of individual.
  ///
  /// We assume that the parents would be ones of the members of the population
  /// closest to the new elements. In this way, children compete with their
  /// parents to be included in the population.
  ///
  /// A child replaces the worst parent if the former has a higher fitness
  /// (_deterministic crowding_ and _elitist recombination_); if elitism is
  /// \c false, the winner of the parent-offspring tournament is chosen by
  /// using a probability proportional to the fitness (_probabistic crowding_).
  ///
  /// \see
  /// "Replacement Strategies to Preserve Useful Diversity in Steady-State
  /// Genetic Algorithms" - Lozano, Herrera, Cano - 2003.
  ///
  template<class T>
  class family_competition : public strategy<T>
  {
  public:
    using family_competition::strategy::strategy;

    virtual void run(const std::vector<coord> &, const std::vector<T> &,
                     summary<T> *const) override;
  };

  ///
  /// \brief Tournament based replacement scheme (aka kill tournament)
  ///
  /// \tparam T type of individual.
  ///
  /// This strategy select an individual for replacement by kill tournament:
  /// pick a number of individuals at random and replace the worst.
  ///
  /// \see
  /// "Replacement Strategies in Steady State Genetic Algorithms: Static
  /// Environments" - Jim Smith, Frank Vavak.
  ///
  template<class T>
  class tournament : public strategy<T>
  {
  public:
    using tournament::strategy::strategy;

    virtual void run(const std::vector<coord> &, const std::vector<T> &,
                     summary<T> *const) override;
  };

  ///
  /// \brief ALPS based replacement scheme
  ///
  /// \tparam T type of individual.
  ///
  /// This strategy select an individual for replacement by an ad hoc kill
  /// tournament.
  /// When an individual is too old for its current layer, it cannot be used
  /// to generate new individuals for that layer and eventually is removed
  /// from that layer. Optionally, an attempt can be made to move this
  /// individual up to the next layer -- in which case it replaces some
  /// individual there that it is better than.
  ///
  /// \see
  /// "Replacement Strategies in Steady State Genetic Algorithms: Static
  /// Environments" - Jim Smith, Frank Vavak.
  ///
  template<class T>
  class alps : public strategy<T>
  {
  public:
    using alps::strategy::strategy;

    virtual void run(const std::vector<coord> &, const std::vector<T> &,
                     summary<T> *const) override;

    void try_move_up_layer(unsigned);

  private:  // Private support methods.
    void try_add_to_layer(unsigned, const T &);
  };

  template<class T>
  class pareto : public strategy<T>
  {
  public:
    using pareto::strategy::strategy;

    virtual void run(const std::vector<coord> &, const std::vector<T> &,
                     summary<T> *const) override;
  };

#include "kernel/evolution_replacement_inl.h"
} }  // namespace vita::replacement

#endif  // EVOLUTION_REPLACEMENT_H
