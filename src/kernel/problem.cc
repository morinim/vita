/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2017 EOS di Manlio Morini.
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
/// \param[in] flag should the environment be initialized with common / default
///                 values?
///
problem::problem(initialization flag) : env(&sset, flag)
{
}

///
/// \return an access point for the data
///
/// \note The default value is `nullptr`: not every problem is dataset-based.
///
vita::data *problem::data()
{
  return nullptr;
}

///
/// \return `true` if the object passes the internal consistency check
///
bool problem::debug() const
{
  if (!env.debug(false))
    return false;

  return sset.debug();
}

}  // namespace vita
