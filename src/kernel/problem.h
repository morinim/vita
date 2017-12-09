/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_PROBLEM_H)
#define      VITA_PROBLEM_H

#include "kernel/data.h"
#include "kernel/environment.h"

namespace vita
{
///
/// Aggregates the problem-related data needed by an evolutionary program.
///
/// \note
/// What a horror! Public data members... please read the coding style
/// document for project Vita.
///
class problem
{
public:
  explicit problem(initialization = initialization::skip);

  virtual vita::data *data();

  virtual bool debug() const;

  template<class T, class... Args> void chromosome(std::size_t, Args && ...);

  environment env;
  symbol_set sset;
};


template<class T, class... Args>
void problem::chromosome(std::size_t length, Args && ...args)
{
  for (decltype(length) i(0); i < length; ++i)
  {
    auto t{std::make_unique<T>(args...)};
    t->category(static_cast<category_t>(i));

    sset.insert(std::move(t));
  }
}

}  // namespace vita

#endif  // include guard
