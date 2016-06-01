/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SATURATION_H)
#define      VITA_SATURATION_H

#include <cmath>
#include <limits>

#include "utility/contracts.h"

namespace vita
{

///
/// \param[in] x a numeric value (a real number in the `[-inf; +inf]` range)
///              that should be mapped in the `[0; 1]` interval.
/// \return a number in the `[0; 1]` range.
///
/// This is a sigmoid function (it's a bounded real function, "S" shaped,
/// with positive derivative everywhere).
/// Among the various uses there's continuous value discretization when we
/// don't know an upper/lower bound for the continuos value.
///
/// \see
/// - <http://en.wikipedia.org/wiki/Sigmoid_function>
/// - <http://en.wikipedia.org/wiki/Generalised_logistic_function>
///
template<class T>
T sigmoid_01(T x)
{
  // See <http://stackoverflow.com/a/23711638/3235496>
  return std::fma(std::atan(x), 0.31830988618, 0.5);  // Arctangent

  // return (1.0 + x / (1 + std::fabs(x))) / 2.0;  // Algebraic function

  // return std::fma(std::tanh(x), 0.5, 0.5);      // Hyperbolic tangent

  // return 1.0 / (1.0 + std::exp(-x));            // Logistic function
}

///
/// \tparam Source type of the input value.
/// \tparam Target type of the output value.
///
/// \param[in] x an input value.
/// \param[in] min minimum value of the saturation range.
/// \param[in] max maximum value of the saturation range.
/// \return the casted and (possibly) saturated value of `x`.
///
/// Partitions the `]-inf, +inf[` range
/// (where `inf = std::numeric_limits<Source>::infinity()`) to a discretized
/// variable in the `[min, max]` interval.
///
/// Discretization of real data into typically small number of finite values is
/// often required by machine learning algorithms, data mining and any
/// modelling algorithm using discrete-state models.
///
template<class Target, class Source>
Target discretization(Source x, Target min, Target max)
{
  static_assert(std::is_floating_point<Source>::value,
                "discretization() requires a floating point input");
  static_assert(std::is_integral<Target>::value,
                "discretization() requires an integral output");

  Expects(min < max);

  const auto ret(std::fma(static_cast<Source>(max - min),
                          sigmoid_01(x),
                          static_cast<Source>(min)));

  Ensures(static_cast<Source>(min) <= ret);
  Ensures(ret <= static_cast<Source>(max));

  return static_cast<Target>(std::round(ret));
}

template<class Target, class Source>
Target discretization(Source x, Target max)
{
  return discretization(x, Target(0), max);
}

}  // namespace vita

#endif  // include guard
