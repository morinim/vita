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

#include "kernel/vita.h"

namespace vita {  namespace replacement {

  ///
  /// \brief The replacement strategy (random, tournament...) for the
  ///        vita::evolution_strategy class.
  ///
  /// In the strategy design pattern, this class is the strategy interface and
  /// \a evolution is the context.
  ///
  /// \see
  /// http://en.wikipedia.org/wiki/Strategy_pattern
  ///
  template<class T>
  class strategy
  {
  public:
    typedef T individual_t;

    strategy(population<T> &, evaluator &);
    virtual ~strategy() {}

    virtual void run(const std::vector<coord> &, const std::vector<T> &,
                     summary<T> *const) = 0;

  protected:
    population<T> &pop_;
    evaluator     &eva_;
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
  class family_competition : public strategy<T>
  {
  public:
    family_competition(population<T> &, evaluator &);

    virtual void run(const std::vector<coord> &, const std::vector<T> &,
                     summary<T> *const) override;
  };

  ///
  /// \brief Tournament based replacement scheme (aka kill tournament).
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
    tournament(population<T> &, evaluator &);

    virtual void run(const std::vector<coord> &, const std::vector<T> &,
                     summary<T> *const) override;
  };

  template<class T>
  class alps : public strategy<T>
  {
  public:
    alps(population<T> &, evaluator &);

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
    pareto(population<T> &, evaluator &);

    virtual void run(const std::vector<coord> &, const std::vector<T> &,
                     summary<T> *const) override;
  };

#include "kernel/evolution_replacement_inl.h"
} }  // namespace vita::replacement

#endif  // EVOLUTION_REPLACEMENT_H
