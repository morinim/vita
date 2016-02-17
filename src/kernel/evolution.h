/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_H)
#define      VITA_EVOLUTION_H

#include <algorithm>
#include <csignal>

#include "kernel/evaluator_proxy.h"
#include "kernel/evolution_strategy.h"
#include "kernel/evolution_summary.h"
#include "kernel/population.h"
#include "kernel/timer.h"

namespace vita
{
///
/// \brief Progressively evolves a population of programs over a series of
///        generations
///
/// \tparam T type of individual
/// \tparam ES evolution strategy in use
///
/// The evolutionary search uses the Darwinian principle of natural selection
/// (survival of the fittest) and analogs of various naturally occurring
/// operations, including crossover (sexual recombination), mutation...
///
template<class T, template<class> class ES>
class evolution
{
public:
  evolution(const environment &, evaluator<T> &,
            std::function<bool (const summary<T> &)> = nullptr,
            std::function<void (unsigned)> = nullptr);

  const summary<T> &run(unsigned);

  bool debug() const;

private:  // Private support methods
  const environment &env() const { return pop_.env(); }
  analyzer<T> get_stats() const;
  void log(unsigned) const;
  void print_progress(unsigned, unsigned, bool, timer *) const;
  bool stop_condition(const summary<T> &) const;

private:  // Private data members
  vita::population<T> pop_;
  evaluator<T>       &eva_;
  summary<T>        stats_;
  ES<T>                es_;

  std::function<bool (const summary<T> &)> external_stop_condition_;
  std::function<void (unsigned)>                        shake_data_;
};

#include "kernel/evolution.tcc"

///
/// \example example6.cc
/// Simple symbolic regression example (\f$x^2+y^2-z^2\f$).
///
}  // namespace vita

#endif  // Include guard
