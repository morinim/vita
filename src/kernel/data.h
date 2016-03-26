/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_DATA_H)
#define      VITA_DATA_H

#include "kernel/vita.h"

namespace vita
{

///
/// \brief Interface for data used by vita::search class to to evolve
///        vita::population.
///
class data
{
public:
  /// Data are stored in three datasets:
  /// - a training set used directly for learning;
  /// - a validation set for controlling overfitting and measuring the
  ///   performance of an individual;
  /// - a test set for a forecast of how well an individual will do in the
  ///   real world.
  /// We don't validate on the training data because that would overfit the
  /// model. We don't stop at the validation step because we've iteratively
  /// been adjusting things to get a winner in the validation step. So we need
  /// an independent test to have an idea of how well we'll do outside the
  /// current arena.
  enum dataset_t {training = 0, validation, test, npos};

  // ---- Constructors ----
  explicit data(dataset_t = npos);

  void select(dataset_t);
  dataset_t active_dataset() const;

  virtual bool has(dataset_t) const = 0;

private:
  dataset_t active_dataset_;
};

}  // namespace vita

#endif  // include guard
