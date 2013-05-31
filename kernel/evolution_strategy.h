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

#include "evolution_operation.h"
#include "evolution_replacement.h"
#include "evolution_selection.h"

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
  ///
  /// Selection, recombination and replacement are the main steps of evolution.
  /// In the literature are described a lot of different algorithms for each of
  /// these steps and many of them are implemented in Vita (but not every
  /// combination is meaningful).
  ///
  /// The user can choose, at compile time, how the evolution class should
  /// work via the evolution strategy class (or one of its specialized alias).
  ///
  /// In other words the template method design pattern is used to "inject"
  /// selection, recombination and replacement methods specified by the
  /// evolution_strategy object into an evolution object.
  ///
  template<class T, class SS, class CS, class RS, class BK>
  class evolution_strategy
  {
  public:
    using selection = SS;
    using recombination = CS;
    using replacement = RS;
    using bookkeeping = BK;

    //enum {
    //  alps =
    //    std::is_same<SS, typename vita::selection::alps<T>>::value &&
    //    std::is_same<RS, typename vita::replacement::alps<T>>::value
    //};
  };

  template<class T>
  class alps_bk
  {
  public:
    static void run(const summary<T> &s, evolution<T> *const e)
    {
      population<T> &pop(e->population());

      pop.inc_age();

      if (s.gen && s.gen % pop.env().alps.age_gap == 0)
      {
        pop.init_layer(0);
        pop.add_layer();
      }
    }
  };

  template<class T>
  using alps_es = evolution_strategy<T, selection::alps<T>, standard_op<T>,
                                     replacement::alps<T>, alps_bk<T>>;


  template<class T>
  class empty_bk
  {
  public:
    static void run(const summary<T> &, evolution<T> *const)
    {
    }
  };

  template<class T>
  using std_es = evolution_strategy<T, selection::tournament<T>,
                                    standard_op<T>,
                                    replacement::tournament<T>, empty_bk<T>>;
}  // namespace vita

#endif  // EVOLUTION_STRATEGY_H
