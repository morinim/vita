/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_STRATEGY_H)
#define      VITA_EVOLUTION_STRATEGY_H

#include "kernel/evolution_recombination.h"
#include "kernel/evolution_replacement.h"
#include "kernel/evolution_selection.h"

namespace vita
{
///
/// Defines the skeleton of the evolution, deferring some steps to client
/// subclasses.
///
/// \tparam T  type of individual
/// \tparam SS class containing the selection strategy
/// \tparam CS class containing the recombination strategy
/// \tparam RS class containing the replacement strategy
///
/// Selection, recombination and replacement are the main steps of evolution.
/// In the literature a lot of different algorithms are described and many of
/// them are implemented in Vita (not every combination is meaningful).
///
/// The user can choose, at compile time, how the evolution class should
/// work via the evolution strategy class (or one of its specialization).
///
/// In other words the template method design pattern is used to "inject"
/// selection, recombination and replacement methods specified by the
/// evolution_strategy object into an `evolution` object.
///
template<class T,
         template<class> class SS,
         template<class> class CS,
         template<class> class RS>
class evolution_strategy
{
public:
  evolution_strategy(population<T> &pop, evaluator<T> &eva, summary<T> *s)
    : selection(pop, eva, *s), recombination(pop, eva, s),
      replacement(pop, eva), pop_(pop), sum_(s)
  {
    Expects(s);
  }

  /// Evolution strategy specific log function (it's called by the
  /// `evolution::log` method).
  void log_strategy(unsigned, unsigned) const {}

  /// Sets strategy-specific parameters.
  /// The default implementation doesn't change the user-specified
  /// environment. Some evolution strategies force parameters to
  /// specific values.
  static environment shape(const environment &env) { return env; }

  /// Initial setup performed before evolution starts.
  void init() const {}

  /// Work to be done at the end of a generation.
  void after_generation() const {}

  /// Used to signal strategy specific stop conditions.
  bool stop_condition() const { return false; }

  static constexpr bool is_alps =
    std::is_same<SS<T>, typename vita::selection::alps<T>>::value &&
    std::is_same<RS<T>, typename vita::replacement::alps<T>>::value;

  static constexpr bool is_de =
    std::is_same<CS<T>, typename vita::recombination::de<T>>::value;

public:
  SS<T> selection;
  CS<T> recombination;
  RS<T> replacement;

protected:
  population<T> &pop_;
  summary<T>    *sum_;
};

///
/// Basic ALPS strategy.
///
/// With ALPS, several instances of a search algorithm are run in parallel,
/// each in its own age-layer, and the age of solutions is kept track of. The
/// key properties of ALPS are:
/// * each age-layer has its own sub-population of one or more candidate
///   solutions (individuals);
/// * each age-layer has a maximum age and it may not contain individuals
///   older than that maximum age;
/// * the age of individuals is based on when the original genetic material
///   was created from random;
/// * the search algorithm in a given age-layer can look at individuals in
///   its own sub-population and at the sub-populations in younger age layers
///   but it can only replace individuals in its own population;
/// * at regular intervals, the search algorithm in the first age-layer is
///   restarted.
///
/// Age is a measure of how long an individual's family of
/// genotypic material has been in the population. Randomly generated
/// individuals, such as those that are created when the search algorithm are
/// started, start with an age of 0. Each generation that an individual stays
/// in the population its age is increased by one. Individuals that are
/// created through mutation or recombination take the age of their oldest
/// parent and add one to it. This differs from conventional measures of age,
/// in which individuals created through applying some type of variation to
/// an existing individual (e.g. mutation or recombination) start with an age
/// of 0.
///
/// The search algorithm in a given layer acts somewhat independently of the
/// others, with an exception being that it can use individuals from both its
/// layer and the layer below to generated new candidate solutions. Also,
/// each age layer has an upper limit on the age of solutions it can contain.
/// When an individual is too old for its current layer, it cannot be used to
/// generate new individuals for that layer and eventually is removed from
/// that layer. Optionally, an attempt can be made to move this individual up
/// to the next layer -- in which case it replaces some individual there that
/// it is better than. Finally, at regular intervals the bottom layer is
/// replaced with a new sub-population of randomly generated individuals,
/// each with an age of 0.
///
/// \see <https://github.com/ghornby/alps>
///
template<class T, template<class> class CS>
class basic_alps_es : public evolution_strategy<T,
                                                selection::alps,
                                                CS,
                                                replacement::alps>
{
public:
  using basic_alps_es::evolution_strategy::evolution_strategy;

  void log_strategy(unsigned, unsigned) const;
  void after_generation();

  static environment shape(environment);
};

template<class T> using alps_es = basic_alps_es<T, recombination::base>;

///
/// Standard evolution strategy.
///
template<class T>
class std_es : public evolution_strategy<T,
                                         selection::tournament,
                                         recombination::base,
                                         replacement::tournament>
{
public:
  using std_es::evolution_strategy::evolution_strategy;

  bool stop_condition() const;

  static environment shape(environment);
};

///
/// Differential evolution strategy.
///
template<class T>
class de_es : public evolution_strategy<T,
                                        selection::random,
                                        recombination::de,
                                        replacement::tournament>
{
public:
  using de_es::evolution_strategy::evolution_strategy;
};

///
/// Differential evolution strategy enhanced with ALPS.
///
/// This is similar to the de_es evolution strategy but more suitable for
/// long/hard searches.
///
template<class T>
class de_alps_es : public basic_alps_es<T, recombination::de>
{
public:
  using de_alps_es::basic_alps_es::basic_alps_es;
};

#include "kernel/evolution_strategy.tcc"
}  // namespace vita

#endif  // include guard
