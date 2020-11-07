/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_EVALUATOR_H)
#define      VITA_GA_EVALUATOR_H

#include "kernel/constrained_evaluator.h"
#include "kernel/vitafwd.h"
#include "kernel/ga/primitive.h"

namespace vita
{
///
/// Calculates the fitness of an individual.
///
/// \note This is a simple adapter for the objective function.
///
/// \warning
/// Being a simple adapter implies that the evolutionary algorithm will try to
/// find out the maximum of the objective function (this derives from our
/// definition of **standardized fitness**).
///
template<class T, class F>
class ga_evaluator : public evaluator<T>
{
public:
  explicit ga_evaluator(F);

  virtual fitness_t operator()(const T &) override;

private:
  // See <https://stackoverflow.com/q/13233213/3235496>
  std::conditional_t<std::is_function_v<F>, std::add_pointer_t<F>, F> f_;
};

template<class T, class F> ga_evaluator<T, F> make_ga_evaluator(F);

#include "kernel/ga/evaluator.tcc"
}  // namespace vita

#endif  // include guard
