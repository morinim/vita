/**
 *
 *  \file score.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SCORE_H)
#define      SCORE_H

#include "kernel/fitness.h"

namespace vita
{
  ///
  /// The fitness and the accuracy (percentage of examples correctly
  /// classified) of an individual.
  ///
  struct score_t
  {
    score_t() {}

    score_t(fitness_t f, double a) : fitness(f), accuracy(a)
    {
      assert(a <= 1.0);  // accuracy could be less than 0.0 (meaning N.A.)
    }

    bool operator==(const score_t &s) const
    { return fitness == s.fitness && accuracy == s.accuracy; }

    fitness_t fitness;
    double   accuracy;
  };
}  // namespace vita

#endif  // SCORE_H
