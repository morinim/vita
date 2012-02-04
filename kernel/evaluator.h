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

#include "kernel/fitness.h"
#include "kernel/random.h"

namespace vita
{
  class individual;

  /// the fitness and the accuracy (percentage) of an individual.
  typedef std::pair<fitness_t, double> eva_pair;

  ///
  /// \a evaluator class calculates the fitness of an individual (how good
  /// he is). It maps vita::individual (its genome) to a fitness_t value.
  /// This is an abstract class because the fitness is domain dependent
  /// (symbolic regression, data classification, automation...).
  /// Note: this class shouldn't be confused with the vita::interpreter
  /// class (that calculates the output of an individual given an input
  /// vector).
  ///
  class evaluator
  {
  public:
    virtual void clear() {}

    /// \return the fitness and the accuracy (percentage) of the individual.
    ///
    /// The accuracy of a program refers to the number of training examples
    /// that are correctly scored/classified as a proportion of the total
    /// number of examples in the training set. According to this
    /// design, the best accuracy is 1.0 (100%), meaning that all the training
    /// examples have been correctly recognized.
    /// Accuracy could be used as fitness function but it often hasn't enough
    /// "granularity".
    virtual eva_pair operator()(const individual &) = 0;
    virtual fitness_t fast(const individual &i) { return operator()(i).first; }
  };

  ///
  /// \a random_evaluator \c class is used for debug purpose.
  /// NOTE: the output is population independent.
  ///
  class random_evaluator : public evaluator
  {
  public:
    eva_pair operator()(const individual &)
    {
      const double sup(16000);
      const fitness_t f(random::between<unsigned>(0, sup));
      return eva_pair(f, f / (sup - 1));
    }
  };
}  // namespace vita

#endif  // EVALUATOR_H
