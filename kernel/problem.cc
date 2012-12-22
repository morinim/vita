/**
 *
 *  \file problem.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "problem.h"
#include "evaluator.h"
#include "search.h"

namespace vita
{
  ///
  /// New empty instance.
  ///
  problem::problem()
  {
    clear();
  }

  ///
  /// Resets the object.
  ///
  void problem::clear()
  {
    env = environment(false);
    active_eva_ = nullptr;
  }

  ///
  /// \return the active evaluator.
  ///
  evaluator_ptr problem::get_evaluator()
  {
    return active_eva_;
  }

  ///
  /// \param[in] e the evaluator that should be set as active.
  ///
  void problem::set_evaluator(const evaluator_ptr &e)
  {
    active_eva_ = e;
  }

  ///
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool problem::check(bool verbose) const
  {
    return env.check(verbose, false);
  }
}  // namespace vita
