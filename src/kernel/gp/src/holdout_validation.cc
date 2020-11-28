/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/gp/src/holdout_validation.h"
#include "kernel/random.h"

namespace vita
{

///
/// Sets up a hold-out validator.
///
/// \param[in] prob current problem
///
holdout_validation::holdout_validation(src_problem &prob)
  : training_(prob.data(dataset_t::training)),
    validation_(prob.data(dataset_t::validation)),
    env_(prob.env)
{
  // Here `env_.validation_percentage.has_value()` could be `false`. Validation
  // strategy is set before parameters are tuned.

  Ensures(validation_.empty());
}

///
/// During the first run examples are randomly partitioned into two sets
/// according to a given percentage.
///
/// \param[in] run current run
///
void holdout_validation::init(unsigned run)
{
  Expects(env_.validation_percentage.has_value());
  Expects(*env_.validation_percentage < 100);
  Expects(!training_.empty());

  if (*env_.validation_percentage == 0)
  {
    vitaWARNING << "Holdout with 0% validation is unusual";
  }

  if (run > 0)  // datasets are set up only one time (at run `0`)
    return;

  assert(validation_.empty());

  const auto perc(*env_.validation_percentage);
  const auto available(training_.size());
  const auto skip(std::max<decltype(available)>(
                    available * (100 - perc) / 100, 1));
  assert(skip <= available);

  // Reservoir sampling via Fisher-Yates shuffling algorithm.
  for (std::size_t i(available - 1); i >= skip; --i)
  {
    auto curr(std::next(training_.begin(), i));
    auto rand(std::next(training_.begin(), random::sup(i + 1)));

    std::iter_swap(curr, rand);
  }

  const auto from(std::next(training_.begin(), skip));
  std::copy(from, training_.end(), std::back_inserter(validation_));
  training_.erase(from, training_.end());

  Ensures(!training_.empty());
  Ensures(training_.size() == skip);
  Ensures(training_.size() + validation_.size() == available);
}

}  // namespace vita
