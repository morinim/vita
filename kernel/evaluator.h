/**
 *
 *  \file evaluator.h
 *
 *  Copyright 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if !defined(EVALUATOR_H)
#define      EVALUATOR_H

#include "kernel/vita.h"
#include "kernel/fitness.h"
#include "kernel/random.h"

namespace vita
{
  class individual;

  ///
  /// \a evaluator \c class calculates the fitness of an individual (how good
  /// he is). It maps an \a individual (its genome) to a \a fitness_t value.
  /// This is an abstract \c class because the fitness is domain dependent
  /// (symbolic regression, data classification, automation...).
  /// Note: this \c class shouldn't be confused with the \a interpreter
  /// \c class (that calculates the output of an individual given an input
  /// vector).
  ///
  class evaluator
  {
  public:
    virtual void clear() {}

    virtual fitness_t operator()(const individual &) = 0;

    virtual double success_rate(const individual &) { return -1; }
  };

  ///
  /// \a random_evaluator \c class is used for debug purpose.
  /// NOTE: the output is population independent.
  ///
  class random_evaluator : public evaluator
  {
  public:
    fitness_t operator()(const individual &)
    {
      return random::between<unsigned>(0, 16000);
    }
  };
}  // namespace vita

#endif  // EVALUATOR_H
