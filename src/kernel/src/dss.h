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

#if !defined(VITA_DSS_H)
#define      VITA_DSS_H

#include "kernel/validation_strategy.h"
#include "kernel/src/problem.h"

namespace vita
{

///
/// Dynamic training Subset Selection.
///
/// When using GP on a difficult problem, with a large set of training data,
/// a large population size is needed and a very large number of evaluations
/// must be carried out. DSS is a subset selection method which uses the
/// available information to select:
/// - firstly 'difficult' cases;
/// - secondly cases which have not been looked at for several generations.
///
/// \see https://github.com/morinim/vita/wiki/bibliography#5
///
class dss : public validation_strategy
{
public:
  explicit dss(src_problem &);

  void init() override;
  bool shake(unsigned) override;

private:
  std::pair<uintmax_t,uintmax_t> average_age_difficulty(dataframe &) const;

  void reset_age_difficulty(dataframe &);
  void shake_impl();

  dataframe &training_;
  dataframe &validation_;
  unsigned gap_;
};

}  // namespace vita

#endif  // include guard
