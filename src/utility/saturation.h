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

template<class T, class F>
bool is_in_range(F value)
{
  if (!std::numeric_limits<T>::is_integer)
    return (value > 0 ? value : -value) <= std::numeric_limits<T>::max();

  if (std::numeric_limits<T>::is_signed == std::numeric_limits<F>::is_signed)
    return value >= std::numeric_limits<T>::lowest() &&
           value <= std::numeric_limits<T>::max();

  if (std::numeric_limits<T>::is_signed)
    return value <= std::numeric_limits<T>::max();

  // std::numeric_limits<F>::is_signed
  // !std::numeric_limits<T>::is_signed
  return value >= 0 && value <= std::numeric_limits<T>::max();
}

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

template<class I, class O>
O saturation(I x, O min, O max)
{
  Expects(max > min);

  using promote_t = decltype(x + max);
  assert(is_in_range<promote_t>(x));
  assert(is_in_range<promote_t>(min));
  assert(is_in_range<promote_t>(max));

  const auto ret(std::fma(static_cast<promote_t>(max - min),
                          sigmoid_01(static_cast<promote_t>(x)),
                          static_cast<promote_t>(min)));

  Ensures(min <= static_cast<O>(ret));
  Ensures(static_cast<O>(ret) <= max);
  return static_cast<O>(ret);
}

template<class I, class O>
O max_saturation(I x, O max)
{
  Expects(max > static_cast<O>(0));

  using promote_t = decltype(x + max);
  assert(is_in_range<promote_t>(x));
  assert(is_in_range<promote_t>(max));

  const auto ret(static_cast<promote_t>(max) *
                 sigmoid_01(static_cast<promote_t>(x)));

  Ensures(static_cast<O>(ret) <= max);
  return static_cast<O>(ret);
}

}  // namespace vita

#endif  // Include guard
