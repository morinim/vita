/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_EVALUATOR_H)
#define      VITA_GA_EVALUATOR_H

#include "kernel/vitafwd.h"
#include "kernel/ga/i_num_ga.h"
#include "kernel/ga/interpreter.h"
#include "kernel/ga/primitive.h"

namespace vita
{
  ///
  /// \brief Calculates the fitness of a i_num_ga individual
  ///
  /// \note
  /// Our convention is to convert raw fitness to standardized fitness. The
  /// requirements for standardized fitness are:
  /// * bigger values represent better choices;
  /// * optimal value is 0.
  ///
  template<>
  class evaluator<i_num_ga>
  {
  public:
    enum clear_flag {cache = 1, stats = 2, all = cache | stats};

    /// \return the fitness of the individual.
    fitness_t operator()(const i_num_ga &);

    /// There isn't a faster but approximated version of the evaluator, so the
    /// implementation calls the standard fitness function.
    fitness_t fast(const i_num_ga &i) { return operator()(i); }

    /// NOT USED: there isn't a training set for differential evolution.
    double accuracy(const i_num_ga &) const { return -1.0; }

    virtual void clear(clear_flag) {}
    virtual void clear(const i_num_ga &) {}
    virtual std::string info() const { return std::string(); }

    // std::unique_ptr<lambda_f<i_num_ga>> lambdify(const i_num_ga &) const
    // { return nullptr; }
  };

  ///
  /// \return the fitness of the individual (range [-1000;0]).
  ///
  inline fitness_t evaluator<i_num_ga>::operator()(const i_num_ga &i)
  {
    interpreter<i_num_ga> it(i);
    const any x(it.run());

    if (x.empty())
      return fitness_t();

    return 1000.0 * (std::atan(vita::to<ga::base_t>(x)) / 3.1415926535 - 0.5);
    //return -std::exp(-vita::to<ga::base_t>(x) / 1000.0);
  }
}  // namespace vita

#endif  // Include guard
