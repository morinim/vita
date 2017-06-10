/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_FIXED_VALIDATION_H)
#define      VITA_FIXED_VALIDATION_H

#include "kernel/validation_strategy.h"
#include "kernel/src/data.h"

namespace vita
{

///
/// \brief Holdout validation, aka one round cross-validation or conventional
///        validation.
///
/// Holdout validation involves partitioning a sample of data into
/// complementary subsets, performing the analysis on one subset (called the
/// training set) and validating the analysis on the other subset (called the
/// validation set).
///
class holdout_validation : public validation_strategy
{
public:
  holdout_validation(src_data &, unsigned);

  void preliminary_setup() override;
  bool shake(unsigned) override { return false; }
  void final_bookkeeping() override {}

private:
  src_data &dat_;
  unsigned perc_;
};

}  // namespace vita

#endif  // include guard
