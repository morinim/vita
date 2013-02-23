/**
 *
 *  \file evaluator.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVALUATOR_H)
#define      EVALUATOR_H

#include "fitness.h"
#include "random.h"

namespace vita
{
  class individual;
  class lambda_f;

  ///
  /// \a evaluator class calculates the fitness of an individual (how good
  /// he is). It maps vita::individual (its genome) to a fitness_t value.
  /// This is an abstract class because the fitness is domain dependent
  /// (symbolic regression, data classification, automation...).
  /// Note: this class shouldn't be confused with the vita::interpreter
  /// class (that calculates the output of an individual given an input
  /// vector).
  ///
  /// Our convention is to convert raw fitness to standardized fitness. The
  /// requirements for standardized fitness are:
  /// \li bigger values represent better choices;
  /// \li optimal value is 0.
  ///
  class evaluator
  {
  public:
    typedef std::shared_ptr<evaluator> ptr;

    virtual void clear() {}
    virtual void clear(const individual &) {}

    /// \return the fitness and the accuracy (percentage) of the individual.
    ///
    /// The accuracy of a program refers to the number of training examples
    /// that are correctly scored/classified as a proportion of the total
    /// number of examples in the training set. According to this
    /// design, the best accuracy is 1.0 (100%), meaning that all the training
    /// examples have been correctly recognized.
    /// Accuracy could be used as fitness function but it sometimes hasn't
    /// enough "granularity".
    virtual fitness_t operator()(const individual &) = 0;

    virtual fitness_t fast(const individual &i) { return operator()(i); }

    virtual std::unique_ptr<lambda_f> lambdify(const individual &) const;
  };

  ///
  /// \a random_evaluator class is used for debug purpose.
  ///
  /// \note
  /// The output is population independent.
  ///
  class random_evaluator : public evaluator
  {
  public:
    static size_t dim;

    virtual fitness_t operator()(const individual &);
  };
}  // namespace vita

#endif  // EVALUATOR_H
