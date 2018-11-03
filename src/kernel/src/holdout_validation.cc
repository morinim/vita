/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/src/holdout_validation.h"
#include "kernel/random.h"

namespace vita
{

///
/// Sets up a hold-out validator.
///
/// \param[in] prob current problem
///
holdout_validation::holdout_validation(src_problem &prob)
  : training_(prob.data(problem::training)),
    validation_(prob.data(problem::validation)),
    env_(prob.env)
{
  // Here `!env_.validation_percentage.has_value()` could be true. Validation
  // strategy is set before parameters are tuned.

  Ensures(validation_.empty());
}

///
/// At the first run examples are randomly partitioned into two sets according
/// to a given percentage.
///
/// \param[in] run current run
///
void holdout_validation::init(unsigned run)
{
  Expects(env_.validation_percentage.has_value());
  Expects(*env_.validation_percentage >   0);
  Expects(*env_.validation_percentage < 100);
  Expects(!training_.empty());

  if (run == 0)  // datasets are set up only one time (at run `0`)
  {
    assert(validation_.empty());

    const auto perc(*env_.validation_percentage);
    const auto available(training_.size());
    const auto skip(available * (100 - perc) / 100);
    assert(skip <= available);

    std::shuffle(training_.begin(), training_.end(), random::engine);

    const auto from(std::next(training_.begin(), skip));
    std::move(from, training_.end(), std::back_inserter(validation_));
    training_.erase(from, training_.end());

    // An alternative is the Selection sampling / Algorithm S (see
    // <http://stackoverflow.com/q/35065764/3235496>)
    //
    // > Iterate through and for each element make the probability of
    // >  selection = (number needed)/(number left)
    // >
    // > So if you had 40 items, the first would have a 5/40 chance of being
    // > selected. If it is, the next has a 4/39 chance, otherwise it has a
    // > 5/39 chance. By the time you get to the end you will have your 5
    // > items, and often you'll have all of them before that.
  }
}

}  // namespace vita
