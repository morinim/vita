/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_ALPS_H)
#define      VITA_ALPS_H

#include "kernel/vita.h"

namespace vita
{
  ///
  /// alps namespace contains some support functions for the ALPS algorithm
  ///
  namespace alps
  {
    unsigned max_age(unsigned, unsigned, unsigned);
  }  // namespace alps
}  // namespace vita

#endif  // Include guard
