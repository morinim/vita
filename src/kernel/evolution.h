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

#if !defined(VITA_EVOLUTION_H)
#define      VITA_EVOLUTION_H

#include <algorithm>
#include <csignal>

#include "kernel/evaluator_proxy.h"
#include "kernel/evolution_strategy.h"
#include "kernel/evolution_summary.h"
#include "kernel/population.h"
#include "utility/timer.h"

namespace vita
{
///
/// Progressively evolves a population of programs over a series of
/// generations.
///
/// \tparam T  type of individual
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
  using after_generation_callback_t = std::function<void(const population<T> &,
                                                         const summary<T> &)>;

  evolution(const problem &, evaluator<T> &);

  evolution &after_generation(after_generation_callback_t);

  const summary<T> &run(unsigned);
  template<class S> const summary<T> &run(unsigned, S);

  bool is_valid() const;

private:
  // *** Support methods ***
  analyzer<T> get_stats() const;
  void log_evolution(unsigned) const;
  void print_progress(unsigned, unsigned, bool, timer *) const;
  bool stop_condition(const summary<T> &) const;

  // *** Data members ***
  population<T> pop_;
  evaluator<T> &eva_;
  summary<T>  stats_;
  ES<T>          es_;

  after_generation_callback_t after_generation_callback_;
};

#include "kernel/evolution.tcc"

///
/// \example example6.cc
/// Simple symbolic regression example (`x^2+y^2-z^2`).
///
}  // namespace vita

#endif  // include guard
