/**
 *
 *  \file evaluator.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "kernel/evaluator.h"
#include "kernel/lambda_f.h"

namespace vita
{
  size_t random_evaluator::dim(2);

  ///
  /// \return \c nullptr.
  ///
 std::unique_ptr<lambda_f> evaluator::lambdify(const individual &) const
  {
    return nullptr;
  }

  ///
  /// \return a random fitness.
  ///
  fitness_t random_evaluator::operator()(const individual &)
  {
    const double sup(16000.0);

    fitness_t f;
    for (size_t i(0); i < fitness_t::size; ++i)
      f[i] = random::sup(sup);

    return f;
  }
}  // namespace vita
