/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
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
problem::problem() : env(), sset()
{
}

///
/// \return `true` if the object passes the internal consistency check
///
bool problem::is_valid() const
{
  return env.is_valid(false);
}

}  // namespace vita
