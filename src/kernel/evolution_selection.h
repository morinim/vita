/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
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

namespace vita::selection
{

///
/// The strategy (tournament, fitness proportional...) for the evolution class.
///
/// \tparam T the type of program (individual/team)
///
/// In the strategy design pattern, this class is the strategy interface and
/// evolution is the context.
///
/// \see <http://en.wikipedia.org/wiki/Strategy_pattern>
///
template<class T>
class strategy
{
public:
  using parents_t = std::vector<typename population<T>::coord>;

  strategy(const population<T> &, evaluator<T> &, const summary<T> &);

protected:
  const population<T> &pop_;
  evaluator<T>        &eva_;
  const summary<T>    &sum_;
};

///
/// Tournament selection is a method of selecting an individual from a
/// population of individuals. It involves running several *tournaments*
/// among a few individuals chosen *at random* from the population. The
/// winner of each tournament (the one with the best fitness) is selected
/// for crossover.
///
/// Selection pressure is easily adjusted by changing the tournament size.
/// If the tournament size is larger, weak individuals have a smaller chance
/// to be selected.
/// A 1-way tournament selection is equivalent to random selection.
///
/// Tournament selection has several benefits: it's efficient to code, works
/// on parallel architectures and allows the selection pressure to be easily
/// adjusted.
///
/// The tournament selection algorithm we implemented was modified so that
/// instead of having only one winner (parent) in each tournament, we select
/// `n` winners from each tournament based on the top `n` fitness values in the
/// tournament.
///
template<class T>
class tournament : public strategy<T>
{
public:
  using tournament::strategy::strategy;

  typename strategy<T>::parents_t run();
};

///
/// Alps selection as described in <https://github.com/ghornby/alps> (see also
/// vita::basic_alps_es for further details).
///
template<class T>
class alps : public strategy<T>
{
public:
  using alps::strategy::strategy;

  typename strategy<T>::parents_t run();

private:
  typename population<T>::coord pickup(unsigned, double = 1.0) const;
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

  typename strategy<T>::parents_t run();

private:
  void front(const std::vector<unsigned> &, std::set<unsigned> *,
             std::set<unsigned> *) const;
};

///
/// Pick a set of random individuals.
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

  typename strategy<T>::parents_t run();
};

#include "kernel/evolution_selection.tcc"

}  // namespace vita::selection

#endif  // include guard
