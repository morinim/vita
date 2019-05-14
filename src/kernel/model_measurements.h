/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_MODEL_MEASUREMENTS_H)
#define      VITA_MODEL_MEASUREMENTS_H

#include "kernel/fitness.h"

namespace vita
{
///
/// A collection of measurements.
///
struct model_measurements
{
  explicit model_measurements(const fitness_t &f = {}, double a = -1.0,
                              bool s = false)
    : fitness(f), accuracy(a), is_solution(s)
  {
    Expects(accuracy <= 1.0);
  }

  fitness_t fitness;
  double   accuracy;
  bool  is_solution;
  //double f1_score = std::numeric_limits<decltype(f1_score)>::quiet_NaN();
};

///
/// \param[in] lhs first term of comparison
/// \param[in] rhs second term of comparision
///
/// \warning
/// This is a partial ordering relation since is somewhat based on Pareto
/// dominance.
///
/// \relates model_measurements
///
inline bool operator>=(const model_measurements &lhs,
                       const model_measurements &rhs)
{
  return dominating(lhs.fitness, rhs.fitness) &&
         lhs.accuracy >= rhs.accuracy;
}

}  // namespace vita

#endif  // include guard
