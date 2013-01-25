/**
 *
 *  \file src_evaluator.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_EVALUATOR_H)
#define      SRC_EVALUATOR_H

#include <vector>

#include "evaluator.h"
#include "lambda_f.h"
#include "primitive/factory.h"

namespace vita
{
  class individual;
  class src_interpreter;

  class src_evaluator : public evaluator
  {
  public:
    explicit src_evaluator(data &);

  protected:
    data *dat_;
  };

  ///
  /// This class models the evaluators that will drive the evolution towards
  /// the minimum sum of some sort of error.
  /// \see \ref sse_evaluator and \ref sae_evaluator.
  ///
  class sum_of_errors_evaluator : public src_evaluator
  {
  public:
    explicit sum_of_errors_evaluator(data &d) : src_evaluator(d) {}

    score_t operator()(const individual &);
    score_t fast(const individual &);
    virtual std::unique_ptr<lambda_f> lambdify(const individual &) const;

  private:
    virtual double error(src_interpreter &, data::example &, int *const,
                         unsigned *const) = 0;
  };

  ///
  /// This evaluator will drive the evolution towards the minimum sum of
  /// absolute errors (\f$\sum_{i=1}^n abs(target_i - actual_i)\f$).
  /// There is also a penality for illegal values (it is a function of the
  /// number of illegal values).
  /// It is interesting to note that the sum of absolute errors is also
  /// minimized in the least absolute deviations (LAD) approach to regression.
  /// LAD is a robust estimation technique in that it is less sensitive to the
  /// presence of outliers than OLS (Ordinary Least Squares), but is less
  /// efficient than OLS when no outliers are present. It is equivalent to
  /// maximum likelihood estimation under a Laplace distribution model for
  /// \f$epsilon\f$ (sampling error).
  /// \see \ref sse_evaluator.
  ///
  class sae_evaluator : public sum_of_errors_evaluator
  {
  public:
    explicit sae_evaluator(data &d) : sum_of_errors_evaluator(d) {}

  private:
    virtual double error(src_interpreter &, data::example &, int *const,
                         unsigned *const);
  };

  ///
  /// This evaluator will drive the evolution towards the minimum sum of
  /// squared errors (\f$\sum_{i=1}^n (target_i - actual_i)^2\f$).
  /// There is also a penality for illegal values (it is a function of the
  /// number of illegal values).
  /// \note Real data always have noise (sampling/measurement errors) and noise
  /// tends to follow a Gaussian distribution. It can be shown that when we
  /// have a bunch of data with errors drawn from such a distribution you are
  /// most likely to find the "correct" underlying model if you seek to
  /// minimize the sum of squared errors.
  /// \see \ref sae_evaluator.
  ///
  class sse_evaluator : public sum_of_errors_evaluator
  {
  public:
    explicit sse_evaluator(data &d) : sum_of_errors_evaluator(d) {}

  private:
    virtual double error(src_interpreter &, data::example &, int *const,
                         unsigned *const);
  };

  ///
  /// This evaluator will drive the evolution towards the maximum sum of
  /// matches (\f$\sum_{i=1}^n target_i == actual_i\f$).
  /// All incorrect answers receive the same fitness penality.
  ///
  class count_evaluator : public sum_of_errors_evaluator
  {
  public:
    explicit count_evaluator(data &d) : sum_of_errors_evaluator(d) {}

  private:
    virtual double error(src_interpreter &, data::example &, int *const,
                         unsigned *const);
  };

  ///
  /// Slotted Dynamic Class Boundary Determination
  ///
  /// Rather than using fixed static thresholds as boundaries to distinguish
  /// between different classes, this approach introduces a method of
  /// classification where the boundaries between different classes can be
  /// dynamically determined during the evolutionary process.
  ///
  /// \see
  /// "Multiclass Object Classification Using Genetic Programming" - CS-RE-04/2
  /// - Mengjie Zhang, Will Smart -
  /// <http://www.mcs.vuw.ac.nz/comp/Publications/CS-TR-04-2.abs.html>
  ///
  class dyn_slot_evaluator : public src_evaluator
  {
  public:
    explicit dyn_slot_evaluator(data &, size_t = 10);

    score_t operator()(const individual &);
    virtual std::unique_ptr<lambda_f> lambdify(const individual &) const;

  private:
    dyn_slot_engine engine_;

    /// Number of slots for each class of the training set.
    size_t x_slot_;
  };

  ///
  /// This evaluator uses Gaussian distribution for multiclass object
  /// classification.
  /// Instead of using predefined multiple thresholds to form different regions
  /// in the program output space for different classes, this approach uses
  /// probabilities of different classes, derived from Gaussian distributions,
  /// to construct the fitness function for classification.
  ///
  /// \see
  /// "Using Gaussian Distribution to Construct Fitness Functions in Genetic
  /// Programming for Multiclass Object Classification" - CS-TR-05-5 - Mangjie
  /// Zhang, Will Smart.
  ///
  class gaussian_evaluator : public src_evaluator
  {
  public:
    explicit gaussian_evaluator(data &d) : src_evaluator(d) {}

    score_t operator()(const individual &);
    virtual std::unique_ptr<lambda_f> lambdify(const individual &) const;

  private:
    gaussian_engine engine_;
  };
}  // namespace vita

#endif  // SRC_EVALUATOR_H
