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

holdout_validation::holdout_validation(src_data &d, unsigned perc)
  : dat_(d), perc_(perc)
{
  Expects(perc < 100);
}

///
/// Available examples are randomly partitioned into two independent sets
/// according to a given percentage.
///
/// \attention The procedure resets current training / validation sets.
///
void holdout_validation::init()
{
  // Validation set items are moved to the training set.
  dat_.move_append(data::validation, data::training);

  const auto available(dat_.size(data::training));
  if (available)
  {
    assert(perc_ < 100);
    const auto needed(available * perc_ / 100);

    dat_.select(data::training);
    std::shuffle(dat_.begin(data::training), dat_.end(data::training),
                 random::engine);

    dat_.move_append(data::training, data::validation, needed);

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
