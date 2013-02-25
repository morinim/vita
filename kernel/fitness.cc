/**
 *
 *  \file fitness.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "fitness.h"

namespace vita
{
  ///
  /// Standard output operator for fitness_t.
  ///
  std::ostream &operator<<(std::ostream &o, const fitness_t &f)
  {
    o << '(';

    for (size_t i(0); i < f.size(); ++i)
    {
      o << f[i];
      if (i + 1 < f.size())
        o << ", ";
    }

    return o << ')';
  }
}  // namespace vita
