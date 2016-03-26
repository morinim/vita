/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2016 EOS di Manlio Morini.
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
/// \brief A new uninitialized problem.
///
/// \param[in] initialize should the environment be initialized with default
///                       values?
///
problem::problem(bool initialize) : env(&sset, initialize)
{
}

///
/// \return an access point for the data.
///
/// The default value is `nullptr`: not every problem is dataset-based.
///
vita::data *problem::data()
{
  return nullptr;
}

///
/// \brief Resets the object.
/// \param[in] initialize if `true` initialize the environment with default
///                       values.
///
void problem::clear(bool initialize)
{
  *this = std::move(problem(initialize));
  //env = environment(&sset, initialize);
  //sset = symbol_set();
}

///
/// \return `true` if the object passes the internal consistency check.
///
bool problem::debug() const
{
  if (!env.debug(false))
    return false;

  return sset.debug();
}

}  // namespace vita
