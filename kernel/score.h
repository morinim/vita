/**
 *
 *  \file score.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SCORE_H)
#define      SCORE_H

#include <iostream>

#include "fitness.h"

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

    friend std::ostream &operator<<(std::ostream &, const score_t &);

    bool operator==(const score_t &s) const
    { return fitness == s.fitness && accuracy == s.accuracy; }

    bool operator!=(const score_t &s) const
    { return fitness != s.fitness || accuracy != s.accuracy; }

    bool operator>(const score_t &s) const
    {
      return
        fitness > s.fitness || (fitness == s.fitness && accuracy > s.accuracy);
    }

    fitness_t fitness;
    double   accuracy;

    ///
    /// \warning
    /// Do not change with a static const variable definition: danger of static
    /// initialization order fiasco.
    ///
    static score_t lowest()
    {
      static const score_t l{std::numeric_limits<fitness_t>::lowest(),
                             std::numeric_limits<double>::lowest()};
      return l;
    }
  };
}  // namespace vita

#endif  // SCORE_H
