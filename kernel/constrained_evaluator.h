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

#if !defined(VITA_CONSTRAINED_EVALUATOR_H)
#define      VITA_CONSTRAINED_EVALUATOR_H

#include "kernel/evaluator.h"

namespace vita
{
  ///
  /// \tparam T the type of individual used
  /// \tparam P penalty function
  ///
  /// The class merges an evaluator with a penalty function into a new combined
  /// evaluator.
  ///
  template<class T>
  class constrained_evaluator : public evaluator<T>
  {
  public:
    using penalty_func_t = std::function<int (const T &)>;

    constrained_evaluator(std::unique_ptr<evaluator<T>>,
                          penalty_func_t);

    virtual fitness_t operator()(const T &) override;
    virtual fitness_t fast(const T &) override;

    virtual double accuracy(const T &) const override;

    virtual std::unique_ptr<lambda_f<T>> lambdify(const T &) const override;

  private:
    // Base evaluator.
    std::unique_ptr<evaluator<T>> eva_;

    // Penalty function.
    penalty_func_t penalty_;
  };

#include "kernel/constrained_evaluator_inl.h"
}  // namespace vita

#endif  // include guard
