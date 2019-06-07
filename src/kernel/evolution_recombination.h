/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_RECOMBINATION_H)
#define      VITA_EVOLUTION_RECOMBINATION_H

#include "kernel/evolution_selection.h"
#include "kernel/population.h"
#include "kernel/vitafwd.h"

namespace vita {
namespace recombination {
///
/// The operation strategy (crossover, recombination, mutation...) adopted in
/// the evolution class.
///
/// \tparam T type of program (individual/team)
///
/// A recombination acts upon sets of individuals to generate offspring
/// (this definition generalizes the traditional mutation and crossover
/// operators).
///
/// Operator application is atomic from the point of view of the
/// evolutionary algorithm and every recombination is applied to a well
/// defined list of individuals, without dependencies upon past history.
///
/// In the strategy design pattern, this class is the strategy interface and
/// vita::evolution is the context.
///
/// This is an abstract class: introduction of new operators or redefinition
/// of existing ones is obtained implementing recombination::strategy.
///
/// \see
/// * <http://en.wikipedia.org/wiki/Strategy_pattern>
///
template<class T>
class strategy
{
public:
  using offspring_t = small_vector<T, 1>;
  using parents_t   = typename selection::strategy<T>::parents_t;

  strategy(const population<T> &, evaluator<T> &, summary<T> *);

protected:
  const population<T> &pop_;
  evaluator<T>        &eva_;
  summary<T>        *stats_;
};

///
/// This class defines the program skeleton of a standard genetic
/// programming crossover plus mutation operation. It's a template method
/// design pattern: one or more of the algorithm steps can be overriden
/// by subclasses to allow differing behaviours while ensuring that the
/// overarching algorithm is still followed.
///
template<class T>
class base : public strategy<T>
{
public:
  using base::strategy::strategy;

  typename strategy<T>::offspring_t run(
    const typename strategy<T>::parents_t &);
};

///
/// This is based on the differential evolution four members crossover.
///
template<class T>
class de : public strategy<T>
{
public:
  using de::strategy::strategy;

  typename strategy<T>::offspring_t run(
    const typename strategy<T>::parents_t &);
};

#include "kernel/evolution_recombination.tcc"
}  // namespace recombination
}  // namespace vita

#endif  // include guard
