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

#if !defined(VITA_EVOLUTION_SELECTION_H)
#define      VITA_EVOLUTION_SELECTION_H

#include <set>

#include "kernel/alps.h"

namespace vita {  namespace selection {

  ///
  /// \brief The strategy (tournament, fitness proportional...) for the
  ///        \a evolution class
  ///
  /// \tparam T the type of individual
  ///
  /// In the strategy design pattern, this class is the strategy interface and
  /// \a evolution is the context.
  ///
  /// \see
  /// * <http://en.wikipedia.org/wiki/Strategy_pattern>
  ///
  template<class T>
  class strategy
  {
  public:
    strategy(const population<T> &, evaluator<T> &, const summary<T> &);
    virtual ~strategy() {}

    virtual std::vector<coord> run() = 0;

  protected:  // Support methods
    coord pickup() const;
    coord pickup(unsigned, double = 1.0) const;

  protected:  // Data members
    const population<T> &pop_;
    evaluator<T>        &eva_;
    const summary<T>    &sum_;
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
  /// The tournament selection algorithm we implemented was modified so that
  /// instead of having only one winner (parent) in each tournament, we select
  /// n winners from each tournament based on the top n fitness values in the
  /// tournament.
  ///
  template<class T>
  class tournament : public strategy<T>
  {
  public:
    using tournament::strategy::strategy;

    virtual std::vector<coord> run() override;
  };

  ///
  /// Alps selection as described in
  /// <http://idesign.ucsc.edu/projects/alps.html> (see also
  /// vita::basic_alps_es for further details).
  ///
  template<class T>
  class alps : public strategy<T>
  {
  public:
    using alps::strategy::strategy;

    virtual std::vector<coord> run() override;
  };

  ///
  /// The idea behind FUSS is that we should focus the selection pressure
  /// towards fitness levels which have relatively few individuals rather than
  /// on the highest fitness levels. In this way fitness levels which are
  /// difficult to reach are throughly explored and on no fitness level does the
  /// population size decrease towards extiction.
  ///
  /// \see
  /// * "Tournament versus fitness uniform selection" - Shane Legg, Marcus
  /// Hutter, Akshat Kumar (Technical Report IDSIA-04-04 March 2004).
  ///
  template<class T>
  class fuss : public strategy<T>
  {
  public:
    using fuss::strategy::strategy;

    virtual std::vector<coord> run() override;
  };

  ///
  /// Pareto tournament selection as described in "Pursuing the Pareto
  /// Paradigm" (Mark Kotanchek, Guido Smits, Ekaterina Vladislavleva).
  ///
  template<class T>
  class pareto : public strategy<T>
  {
  public:
    using pareto::strategy::strategy;

    virtual std::vector<coord> run() override;

  private:
    void front(const std::vector<unsigned> &, std::set<unsigned> *,
               std::set<unsigned> *) const;
  };

  ///
  /// \brief Pick a set of random individuals.
  ///
  /// Very simple selection strategy: pick a set of random individuals. The
  /// environment::tournamnet_size property controls the cardinality of the
  /// set.
  ///
  template<class T>
  class random : public strategy<T>
  {
  public:
    using random::strategy::strategy;

    virtual std::vector<coord> run() override;
  };

#include "kernel/evolution_selection.tcc"

} } // namespace vita :: selection

#endif  // Include guard
