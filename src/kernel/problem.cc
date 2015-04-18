/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/problem.h"

namespace vita
{
///
/// A new uninitialized problem.
///
problem::problem()
{
}

///
/// \param[in] initialize if `true` initialize the environment with default
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
/// \return an access point for the dataset.
///
/// The default value is `nullptr`. Not every problem is dataset-based.
///
vita::data *problem::data()
{
  return nullptr;
}

///
/// \param[in] verbose if `true` prints error messages to `std::cerr`.
/// \return `true` if the object passes the internal consistency check.
///
bool problem::debug(bool verbose) const
{
  if (!env.debug(verbose, false))
    return false;

  return sset.debug(verbose);
}

}  // namespace vita
