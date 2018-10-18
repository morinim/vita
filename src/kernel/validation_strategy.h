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

#if !defined(VITA_VALIDATION_STRATEGY_H)
#define      VITA_VALIDATION_STRATEGY_H

#include "kernel/common.h"

namespace vita
{

///
/// Interface for specific training / cross validation techniques (e.g.
/// one round cross validation, dynamic subsect selection...).
///
class validation_strategy
{
public:
  virtual ~validation_strategy() = default;

  /// Initializes the data structures needed for the validation strategy.
  ///
  /// \param[in] run current run
  ///
  /// \note Called at the beginning of the evolution.
  virtual void init(unsigned /* run */) {}

  /// Changes the training environment.
  ///
  /// \param[in] generation current generation
  /// \return    `true` if some change in the training environment has occured
  ///
  /// \note Called at the beginning of every generation.
  virtual bool shake(unsigned /* generation */) = 0;

  /// De-initializes the data structures needed for the validation strategy.
  ///
  /// \param[in] run current run
  ///
  /// \note Called at the end of the evolution.
  virtual void close(unsigned /* run */) {}
};

///
/// A "null object" implementation of validation_strategy.
///
/// Implements the interface of a `validation_strategy` with empty body
/// methods (it's very predictable and has no side effects: it does nothing).
///
/// \see <https://en.wikipedia.org/wiki/Null_Object_pattern>
///
class as_is_validation final : public validation_strategy
{
public:
  /// Does nothing, signalling that anything is changed.
  bool shake(unsigned) override { return false; }
};

}  // namespace vita

#endif  // include guard
