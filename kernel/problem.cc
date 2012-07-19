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

#include "kernel/problem.h"
#include "kernel/evaluator.h"
#include "kernel/search.h"

namespace vita
{
  ///
  /// New empty instance.
  ///
  problem::problem() : env(false)
  {
    clear();
  }

  ///
  /// Resets the object.
  ///
  void problem::clear()
  {
    env.sset = vita::symbol_set();
    active_eva_.reset();
    evaluators_.clear();
    threashold = {boost::none, boost::none};
  }

  ///
  /// \return the active evaluator.
  ///
  evaluator *problem::get_evaluator()
  {
    return active_eva_.get();
  }

  ///
  /// \param[in] eva a pointer to an evaluator.
  /// \return numeric index of the just added evaluator.
  ///
  /// Add a new avaluator to the set. Evaluators are used to score individual's
  /// fitness / accuracy.
  ///
  unsigned problem::add_evaluator(evaluator_ptr eva)
  {
    evaluators_.push_back(eva);

    if (!active_eva_)
      active_eva_ = eva;

    return evaluators_.size() - 1;
  }

  ///
  /// \param[in] i index of the evaluator that should be set as active.
  ///
  void problem::set_evaluator(unsigned i)
  {
    active_eva_ = evaluators_[i];
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
