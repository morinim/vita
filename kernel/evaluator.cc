/**
 *
 *  \file evaluator.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "evaluator.h"
#include "lambda_f.h"

namespace vita
{
  ///
  /// \return \c nullptr.
  ///
 std::unique_ptr<lambda_f> evaluator::lambdify(const individual &) const
  {
    return nullptr;
  }

  ///
  /// \return a random score.
  ///
  score_t random_evaluator::operator()(const individual &)
  {
    const double sup(16000.0);
    const fitness_t f(random::between<unsigned>(0, sup));
    return score_t(f, f / (sup - 1.0));
  }
}  // namespace vita
