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

#include "kernel/evaluator.h"
#include "kernel/vitafwd.h"
#include "kernel/ga/primitive.h"

namespace vita
{
  ///
  /// \brief Calculates the fitness of a i_ga individual
  ///
  /// \note
  /// Our convention is to convert raw fitness to standardized fitness. The
  /// requirements for standardized fitness are:
  /// * bigger values represent better choices;
  /// * optimal value is 0.
  ///
  template<class T, class F>
  class ga_evaluator : public evaluator<T>
  {
  public:
    explicit ga_evaluator(F);

    virtual fitness_t operator()(const T &) override;

  private:  // Private data members
    F f_;
  };

  template<class T, class F> ga_evaluator<T, F> make_evaluator(F);

#include "kernel/ga/evaluator_inl.h"
}  // namespace vita

#endif  // Include guard
