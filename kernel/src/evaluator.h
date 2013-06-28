/**
 *
 *  \file src/evaluator.h
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

#include "kernel/evaluator.h"
#include "kernel/lambda_f.h"
#include "kernel/src/primitive/factory.h"

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
  /// \brief An evaluator to minimize the sum of some sort of error.
  ///
  /// This class models the evaluators that will drive the evolution towards
  /// the minimum sum of some sort of error.
  /// \see mse_evaluator, mae_evaluator, rmae_evaluator.
  ///
  class sum_of_errors_evaluator : public src_evaluator
  {
  public:
    explicit sum_of_errors_evaluator(data &d) : src_evaluator(d) {}

    virtual fitness_t operator()(const individual &) override;
    virtual fitness_t fast(const individual &) override;
    virtual std::unique_ptr<lambda_f> lambdify(
      const individual &) const override;

    virtual double accuracy(const individual &) const override;

  private:
    virtual double error(src_interpreter &, data::example &, int *const) = 0;
  };

  ///
  /// \brief Evaluator based on the mean absolute error
  ///
  /// This evaluator will drive the evolution towards the minimum sum of
  /// absolute errors(\f$\frac{1}{n} \sum_{i=1}^n |target_i - actual_i|\f$).
  ///
  /// There is also a penality for illegal values (it is a function of the
  /// number of illegal values).
  ///
  /// \note
  /// When the dataset contains outliers, the mse_evaluator will heavily
  /// weight each of them (this is the result of squaring the outliers).
  /// mae_evaluator is less sensitive to the presence of outliers (a
  /// desiderable property in many application).
  ///
  /// \see \ref mse_evaluator.
  ///
  class mae_evaluator : public sum_of_errors_evaluator
  {
  public:
    explicit mae_evaluator(data &d) : sum_of_errors_evaluator(d) {}

  private:
    virtual double error(src_interpreter &, data::example &, int *const);
  };

  ///
  /// \brief Evaluator based on the mean of relative differences
  ///
  /// This evaluator will drive the evolution towards the minimum sum of
  /// relative differences between target values and actual ones:
  ///
  /// \f[\frac{1}{n} \sum_{i=1}^n \frac{|target_i - actual_i|}{\frac{|target_i| + |actual_i|}{2}}\f]
  ///
  /// This is similar to mae_evaluator but here we sum the _relative_ errors.
  /// The idea is that the absolute difference of 1 between 6 and 5 is more
  /// significant than the same absolute difference between 1000001 and
  /// 1000000.
  /// The mathematically precise way to express this notion is to
  /// calculate the relative difference.
  ///
  /// \see
  /// * <http://realityisvirtual.com/book2/?p=81>
  /// * <http://en.wikipedia.org/wiki/Relative_difference>
  ///
  class rmae_evaluator : public sum_of_errors_evaluator
  {
  public:
    explicit rmae_evaluator(data &d) : sum_of_errors_evaluator(d) {}

  private:
    virtual double error(src_interpreter &, data::example &, int *const);
  };

  ///
  /// \brief Evaluator based on the mean squared error
  ///
  /// This evaluator will drive the evolution towards the minimum sum of
  /// squared errors (\f$\frac{1}{n} \sum_{i=1}^n (target_i - actual_i)^2\f$).
  ///
  /// There is also a penality for illegal values (it is a function of the
  /// number of illegal values).
  ///
  /// \note
  /// Real data always have noise (sampling/measurement errors) and noise
  /// tends to follow a Gaussian distribution. It can be shown that when we
  /// have a bunch of data with errors drawn from such a distribution you are
  /// most likely to find the "correct" underlying model if you seek to
  /// minimize the sum of squared errors.
  ///
  /// \see mae_evaluator.
  ///
  class mse_evaluator : public sum_of_errors_evaluator
  {
  public:
    explicit mse_evaluator(data &d) : sum_of_errors_evaluator(d) {}

  private:
    virtual double error(src_interpreter &, data::example &, int *const);
  };

  ///
  /// \brief Evaluator based on the number of matches
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
    virtual double error(src_interpreter &, data::example &, int *const);
  };

  ///
  /// This class is used to factorized out some code of the classification
  /// evaluators.
  ///
  class classification_evaluator : public src_evaluator
  {
  public:
    explicit classification_evaluator(data &d) : src_evaluator(d) {}

    virtual double accuracy(const individual &) const override;
  };

  ///
  /// \brief Slotted Dynamic Class Boundary Determination
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
  class dyn_slot_evaluator : public classification_evaluator
  {
  public:
    explicit dyn_slot_evaluator(data &, size_t = 10);

    virtual fitness_t operator()(const individual &) override;
    virtual std::unique_ptr<lambda_f> lambdify(
      const individual &) const override;

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
  class gaussian_evaluator : public classification_evaluator
  {
  public:
    explicit gaussian_evaluator(data &d) : classification_evaluator(d) {}

    virtual fitness_t operator()(const individual &) override;
    virtual std::unique_ptr<lambda_f> lambdify(
      const individual &) const override;

  private:
    gaussian_engine engine_;
  };

  ///
  /// Single class evaluator for classification problems
  ///
  class binary_evaluator : public classification_evaluator
  {
  public:
    explicit binary_evaluator(data &d) : classification_evaluator(d) {}

    virtual fitness_t operator()(const individual &) override;
    virtual std::unique_ptr<lambda_f> lambdify(
      const individual &) const override;
  };
}  // namespace vita

#endif  // SRC_EVALUATOR_H
