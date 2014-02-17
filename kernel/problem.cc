/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/evaluator_proxy.h"
#include "kernel/problem.h"
#include "kernel/search.h"

namespace vita
{
  ///
  /// \param[in] initialize if \c true initialize the environment with default
  ///                       values.
  ///
  /// New empty instance.
  ///
  problem::problem(bool initialize) : env(initialize)
  {
  }

  ///
  /// \param[in] initialize if \c true initialize the environment with default
  ///                       values.
  ///
  /// Resets the object.
  ///
  void problem::clear(bool initialize)
  {
    env = environment(initialize);
    sset = symbol_set();
  }

  ///
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool problem::debug(bool verbose) const
  {
    if (!env.debug(verbose, false))
      return false;

    return sset.debug();
  }
}  // namespace vita
