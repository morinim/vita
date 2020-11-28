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

#if !defined(VITA_FIXED_VALIDATION_H)
#define      VITA_FIXED_VALIDATION_H

#include "kernel/validation_strategy.h"
#include "kernel/gp/src/problem.h"

namespace vita
{

///
/// Holdout validation, aka *one round cross-validation* or *conventional
/// validation*.
///
/// Holdout validation involves partitioning a sample of data into
/// complementary subsets, performing the analysis on one subset (called the
/// training set) and validating the analysis on the other subset (called the
/// validation set).
///
class holdout_validation : public validation_strategy
{
public:
  explicit holdout_validation(src_problem &);

  void init(unsigned) override;

private:
  dataframe &training_;
  dataframe &validation_;

  const environment &env_;
};

}  // namespace vita

#endif  // include guard
