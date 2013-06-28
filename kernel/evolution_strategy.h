/**
 *
 *  \file evolution_strategy.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_STRATEGY_H)
#define      EVOLUTION_STRATEGY_H

#include "kernel/evolution_recombination.h"
#include "kernel/evolution_replacement.h"
#include "kernel/evolution_selection.h"

namespace vita
{
  ///
  /// \brief A class to specify the evolution steps.
  ///
  /// \tparam T type of the elements of the population (individuals).
  /// \tparam SS class containing the selection strategy
  /// \tparam CS class containing the recombination strategy
  /// \tparam RS class containing the replacement strategy
  ///
  /// Selection, recombination and replacement are the main steps of evolution.
  /// In the literature a lot of different algorithms are described and many of
  /// them are implemented in Vita (but not every combination is meaningful).
  ///
  /// The user can choose, at compile time, how the evolution class should
  /// work via the evolution strategy class (or one of its specialization).
  ///
  /// In other words the template method design pattern is used to "inject"
  /// selection, recombination and replacement methods specified by the
  /// evolution_strategy object into an evolution object.
  ///
  template<class T, class SS, class CS, class RS>
  class evolution_strategy
  {
  public:
    evolution_strategy(evolution<T> *const e, summary<T> *const s)
      : selection(e), recombination(e, s), replacement(e), evo_(e), sum_(s)
    {
      assert(e);
      assert(s);
    }

    /// Initial setup before evolution starts.
    virtual void pre_bookkeeping() {}

    /// Work to be done at the end of an evolution run.
    virtual void post_bookkeeping() {}

    enum
    {
      is_alps =
        std::is_same<SS, typename vita::selection::alps<T>>::value &&
        std::is_same<RS, typename vita::replacement::alps<T>>::value
    };

  public:  // Public data members.
    SS selection;
    CS recombination;
    RS replacement;

  protected:  // Protected data members.
    evolution<T> *evo_;
    summary<T> *sum_;
  };

  ///
  /// \brief ALPS evolution strategy
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
  /// Age (individual::age) is a measure of how long an individual's family of
  /// genotypic material has been in the population. Randomly generated
  /// individuals, such as those that are created when the search algorithm are
  /// started, start with an age of 0. Each generation that an individual stays
  /// in the population its age is increased by one. Individuals that are
  /// created through mutation or recombination take the age of their oldest
  /// parent and add one to it. This differs from conventional measures of age,
  /// in which individuals created through applying some type of variation to
  /// an existing individual (eg mutation or recombination) start with an age
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
  /// \see <http://idesign.ucsc.edu/projects/alps.html>
  ///
  template<class T>
  class basic_alps_es : public evolution_strategy<T,
                                                  selection::alps<T>,
                                                  recombination::base<T>,
                                                  replacement::alps<T>>
  {
  public:
    basic_alps_es(evolution<T> *const e, summary<T> *const s) :
      evolution_strategy<T, selection::alps<T>, recombination::base<T>,
                         replacement::alps<T>>(e, s)
    {}

    virtual void post_bookkeeping() override
    {
      auto &pop(this->evo_->population());

      pop.inc_age();

      if (this->sum_->gen && this->sum_->gen % pop.env().alps.age_gap == 0)
      {
        if (pop.layers() < pop.env().layers)
          pop.add_layer();
        else
        {
          this->replacement.try_move_up_layer(0);
          pop.init_layer(0);
        }
      }
    }
  };

  using alps_es = basic_alps_es<vita::individual>;

  ///
  /// \brief Standard evolution strategy
  ///
  template<class T>
  class basic_std_es : public evolution_strategy<T,
                                                 selection::tournament<T>,
                                                 recombination::base<T>,
                                                 replacement::tournament<T>>
  {
  public:
    basic_std_es(evolution<T> *const e, summary<T> *const s) :
      evolution_strategy<T, selection::tournament<T>, recombination::base<T>,
                         replacement::tournament<T>>(e, s)
    {}
  };

  using std_es = basic_std_es<vita::individual>;
}  // namespace vita

#endif  // EVOLUTION_STRATEGY_H
