/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/ga/problem.h"

namespace vita
{

///
/// Sets up a GA problem for which a solution has the given number of
/// (uniform, same range) parameters.
///
/// \param[in] parameters number of parameters (aka genes in the chromosome)
/// \param[in] range      a half-open interval (the value of each parameter
///                       falls within this range)
///
/// The typical solution of a combinatorial problem can often be represented as
/// a sequence of integers in a given range (and this is the *raison d'etre* of
/// this constructor).
///
ga_problem::ga_problem(std::size_t parameters, const range_t<int> &range)
  : problem()
{
  Expects(sset.categories() == 0);

  for (decltype(parameters) i(0); i < parameters; ++i)
    insert(range);
}

///
/// Sets up a GA problem for which a solution has the given number of
/// (uniform but **not** same range) parameters.
///
/// \param[in] ranges a sequence of half-open intervals (one for each
///                   parameter)
///
/// This is a more flexible form of the other constructor. Each parameter has
/// its own range.
///
ga_problem::ga_problem(const std::vector<range_t<int>> &ranges)
  : problem()
{
  Expects(sset.categories() == 0);

  for (const auto &r : ranges)
    insert(r);
}

///
/// Sets up a DE problem for which a solution has the given number of
/// (uniform, same range) parameters.
///
/// \param[in] parameters number of parameters (aka genes in the chromosome)
/// \param[in] range      a half-open interval (the value of each parameter
///                       falls within this range)
///
/// The typical solution of a numerical optimization problem can often be
/// represented as a sequence of real numbers in a given range (and this is the
/// *raison d'etre* of this constructor).
///
de_problem::de_problem(std::size_t parameters, const range_t<double> &range)
  : problem()
{
  Expects(sset.categories() == 0);

  for (decltype(parameters) i(0); i < parameters; ++i)
    insert(range);
}

///
/// Sets up a DE problem for which a solution has the given number of
/// (uniform but **not** same range) parameters.
///
/// \param[in] ranges a sequence of half-open intervals (one for each
///                   parameter)
///
/// This is a more flexible form of the other constructor. Each parameter has
/// its own range.
///
de_problem::de_problem(const std::vector<range_t<double>> &ranges)
  : problem()
{
  Expects(sset.categories() == 0);

  for (const auto &r : ranges)
    insert(r);
}

}  // namespace vita
