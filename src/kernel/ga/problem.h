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

#if !defined(VITA_GA_PROBLEM_H)
#define      VITA_GA_PROBLEM_H

#include "kernel/problem.h"
#include "kernel/range.h"
#include "kernel/ga/primitive.h"

namespace vita
{

///
/// Provides a GA-specific interface to the generic `problem` class.
///
/// The class is a facade that provides a simpler interface to represent
/// GA-specific problems.
///
class ga_problem : public problem
{
public:
  ga_problem() = default;
  ga_problem(std::size_t, const range_t<int> &);
  explicit ga_problem(const std::vector<range_t<int>> &);

  template<class... Args> terminal *insert(Args &&...);
};

template<class... Args> terminal *ga_problem::insert(Args &&... args)
{
  return static_cast<terminal *>(
    sset.insert<ga::integer>(std::forward<Args>(args)...));
}


///
/// Provides a DE-specific interface to the generic `problem` class.
///
/// The class is a facade that provides a simpler interface to represent
/// DE-specific problems.
///
class de_problem : public problem
{
public:
  de_problem() = default;
  de_problem(std::size_t, const range_t<double> &);
  explicit de_problem(const std::vector<range_t<double>> &);

  template<class... Args> terminal *insert(Args &&...);
};

template<class... Args> terminal *de_problem::insert(Args &&... args)
{
  return static_cast<terminal *>(
    sset.insert<ga::real>(std::forward<Args>(args)...));
}

}  // namespace vita

#endif  // include guard
