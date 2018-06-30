/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_PROBLEM_H)
#define      VITA_PROBLEM_H

#include "kernel/environment.h"

namespace vita
{
///
/// Aggregates the problem-related data needed by an evolutionary program.
///
class problem
{
public:
  explicit problem(initialization = initialization::skip);

  virtual bool debug() const;

  template<class T, class... Args> void chromosome(std::size_t, Args &&...);

  /// Data/simulations are categorised in three sets:
  /// - *training* used directly for learning;
  /// - *validation* for controlling overfitting and measuring the performance
  ///   of an individual;
  /// - *test* for a forecast of how well an individual will do in the real
  ///   world.
  /// The `vita::search` class asks the `problem` class to setup the requested
  /// simulation/dataset via the `select` function.
  enum dataset_t {training = 0, validation, test};

  void select(dataset_t);
  dataset_t active_dataset() const;
  virtual bool has(dataset_t) const;

  // What a horror! Public data members... please read the coding style
  // document for project Vita.
  environment env;
  symbol_set sset;

private:
  virtual void select_impl(dataset_t) {}

  dataset_t active_dataset_;
};


template<class T, class... Args>
void problem::chromosome(std::size_t length, Args &&... args)
{
  Expects(sset.categories() == 0);

  for (decltype(length) i(0); i < length; ++i)
    sset.insert<T>(std::forward<Args>(args)...);
}

}  // namespace vita

#endif  // include guard
