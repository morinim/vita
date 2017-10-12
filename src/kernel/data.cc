/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/data.h"

namespace vita
{

///
/// New empty data instance.
///
data::data(dataset_t d) : active_dataset_(d)
{
}

///
/// Activates the dataset we want to operate on (training / validation /
/// test set).
///
/// \param[in] d the active dataset
///
void data::select(dataset_t d)
{
  active_dataset_ = d;
}

///
/// \return the type (training, validation, test) of the active dataset
///
data::dataset_t data::active_dataset() const
{
  return active_dataset_;
}

}  // namespace vita
