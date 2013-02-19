/**
 *
 *  \file score.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "score.h"

namespace vita
{
  ///
  /// Standard output operator for score_t struct.
  ///
  std::ostream &operator<<(std::ostream &o, const score_t &s)
  {
    o << '(' << s.fitness;

    if (s.accuracy >= 0.0)
      o << ", " << 100.0 * s.accuracy << "%";

    return o << ')';
  }
}  // namespace vita
