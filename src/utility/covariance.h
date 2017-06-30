/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_COVARIANCE_H)
#define      VITA_COVARIANCE_H

#include <cmath>

#include "kernel/vita.h"

namespace vita
{
///
/// The *covariance* is a measure of how much two random variables change
/// together.
///
/// If the greater values of one variable mainly correspond with the greater
/// values of the other variable, and the same holds for the smaller values,
/// i.e. the variables tend to show similar behaviour, the covariance is a
/// positive number. In the opposite case, when the greater values of one
/// variable mainly correspond to the smaller values of the other, i.e. the
/// variables tend to show opposite behaviour, the covariance is negative. The
/// sign of the covariance therefore shows the tendency in the linear
/// relationship between the variables. The magnitude of the covariance is not
/// that easy to interpret (the normalized version of the covariance, the
/// correlation coefficient, however, shows by its magnitude the strength of
/// the linear relation).
///
template<class T = double>
class covariance
{
public:
  covariance();

  void clear();

  void add(T, T);

  bool ::debug() const;

  T cov;

private:
  void update(T, T);

  std::uintmax_t count;

  T v1_avg;
  T v2_avg;
};

///
/// Just the initial setup.
///
template<class T>
covariance<T>::covariance() : cov(0.0), count(0), v1_avg(0.0), v2_avg(0.0)
{
}

///
/// Resets gathered statics.
///
template<class T>
void distribution<T>::clear()
{
  *this = {};
}

///
/// \param[in] v1 new value upon which covariance is recalculated
/// \param[in] v2 new value upon which covariance is recalculated
///
template<class T> void covariance<T>::add(T v1, T v2)
{
  ++count;

  update(v1, v2);
}

///
/// Calculates running covariance of two random variables.
///
/// \param[in] v1 new value upon which covariance is recalculated
/// \param[in] v2 new value upon which covariance is recalculated
///
/// The algorithm used is stable and one-pass.
///
template<class T>
void covariance<T>::update(T v1, T v2)
{
  v1_avg = v1_avg + (v1 - v1_avg) / static_cast<T>(count);

  cov += (v1 - v1_avg) * (v2 - v2_avg);

  v2_avg = v2_avg + (v2 - v2_avg) / static_cast<T>(count);
}

}  // namespace vita

#endif  // include guard
