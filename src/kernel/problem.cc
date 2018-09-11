/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2018 EOS di Manlio Morini.
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
/// A new problem.
///
problem::problem() : env(), sset(), active_dataset_(training)
{
}

///
/// Activates the dataset/simulation we want to operate on.
///
/// \param[in] d the active dataset
///
void problem::select(dataset_t d)
{
  active_dataset_ = d;

  select_impl(d);
}

///
/// \return the type (training, validation, test) of the active
///         dataset / simulation
///
problem::dataset_t problem::active_dataset() const
{
  return active_dataset_;
}

///
/// Asks if a dataset/simulation of a specific kind is available.
///
/// \param[in] d dataset/simulation type
/// \return      `true` if dataset/simulation of type `d` is available
///
/// A training dataset/simulation should always be available.
///
bool problem::has(dataset_t d) const
{
  return d == training;
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
