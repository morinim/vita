/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_EVALUATOR_H)
#define      VITA_SRC_EVALUATOR_H

#include "kernel/evaluator.h"

namespace vita
{
///
/// An evaluator specialized for symbolic regression / classification problems.
///
/// \tparam T  type of individual
/// \tparam DS type of the dataset
///
/// This specialization of the evaluator class is "dataset-aware". It's useful
/// to group common factors of more specialized symbolic regression or
/// classification classes.
///
template<class T, class DS = dataframe>
class src_evaluator : public evaluator<T>
{
public:
  explicit src_evaluator(DS &);

protected:
  DS *dat_;
};

///
/// An evaluator to minimize the sum of some sort of error.
///
/// This class models the evaluators that will drive the evolution towards the
/// minimum sum of some sort of error.
///
/// \see mse_evaluator, mae_evaluator, rmae_evaluator.
///
template<class T, class DS = dataframe>
class sum_of_errors_evaluator : public src_evaluator<T, DS>
{
public:
  explicit sum_of_errors_evaluator(DS &d) : src_evaluator<T>(d) {}

  fitness_t operator()(const T &) override;
  fitness_t fast(const T &) override;
  std::unique_ptr<basic_lambda_f> lambdify(const T &) const override;

private:
  virtual double error(const basic_reg_lambda_f<T, false> &,
                       typename DS::example &, int *) = 0;
};

///
/// Evaluator based on the mean absolute error.
///
/// This evaluator will drive the evolution towards the minimum sum of
/// absolute errors(\f$\frac{1}{n} \sum_{i=1}^n |target_i - actual_i|\f$).
///
/// There is also a penalty for illegal values (it is a function of the
/// number of illegal values).
///
/// \note
/// When the dataset contains outliers, the mse_evaluator will heavily
/// weight each of them (this is the result of squaring the outliers).
/// mae_evaluator is less sensitive to the presence of outliers (a
/// desirable property in many application).
///
/// \see mse_evaluator.
///
template<class T>
class mae_evaluator : public sum_of_errors_evaluator<T>
{
public:
  explicit mae_evaluator(dataframe &d) : sum_of_errors_evaluator<T>(d) {}

private:
  double error(const basic_reg_lambda_f<T, false> &, dataframe::example &,
               int *) override;
};

///
/// Evaluator based on the mean of relative differences.
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
/// \see https://github.com/morinim/documents/blob/master/math_notes/relative_difference.md
///
template<class T>
class rmae_evaluator : public sum_of_errors_evaluator<T>
{
public:
  explicit rmae_evaluator(dataframe &d) : sum_of_errors_evaluator<T>(d) {}

private:
  double error(const basic_reg_lambda_f<T, false> &, dataframe::example &,
               int *) override;
};

///
/// Evaluator based on the mean squared error.
///
/// This evaluator will drive the evolution towards the minimum sum of
/// squared errors (\f$\frac{1}{n} \sum_{i=1}^n (target_i - actual_i)^2\f$).
///
/// There is also a penalty for illegal values (it is a function of the
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
template<class T>
class mse_evaluator : public sum_of_errors_evaluator<T>
{
public:
  explicit mse_evaluator(dataframe &d) : sum_of_errors_evaluator<T>(d) {}

private:
  double error(const basic_reg_lambda_f<T, false> &, dataframe::example &,
               int *) override;
};

///
/// Evaluator based on the number of matches.
///
/// This evaluator will drive the evolution towards the maximum sum of
/// matches (\f$\sum_{i=1}^n target_i == actual_i\f$).
/// All incorrect answers receive the same fitness penalty.
///
template<class T>
class count_evaluator : public sum_of_errors_evaluator<T>
{
public:
  explicit count_evaluator(dataframe &d) : sum_of_errors_evaluator<T>(d) {}

private:
  double error(const basic_reg_lambda_f<T, false> &, dataframe::example &,
               int *) override;
};

///
/// This class is used to factorized out some code of the classification
/// evaluators.
///
template<class T>
class classification_evaluator : public src_evaluator<T>
{
public:
  explicit classification_evaluator(dataframe &d) : src_evaluator<T>(d) {}
};

///
/// Slotted Dynamic Class Boundary Determination.
///
/// Rather than using fixed static thresholds as boundaries to distinguish
/// between different classes, this approach introduces a method of
/// classification where the boundaries between different classes can be
/// dynamically determined during the evolutionary process.
///
/// \see
/// [Multiclass Object Classification Using Genetic Programming](https://github.com/morinim/vita/wiki/bibliography#12)
///
template<class T>
class dyn_slot_evaluator : public classification_evaluator<T>
{
public:
  explicit dyn_slot_evaluator(dataframe &, unsigned = 10);

  fitness_t operator()(const T &) override;
  std::unique_ptr<basic_lambda_f> lambdify(const T &) const override;

private:
  /// Number of slots for each class of the training set.
  unsigned x_slot_;
};

///
/// Gaussian distribution for multiclass object classification.
///
/// Instead of using predefined multiple thresholds to form different regions
/// in the program output space for different classes, this approach uses
/// probabilities of different classes, derived from Gaussian distributions,
/// to construct the fitness function for classification.
///
/// \see
/// [Using Gaussian Distribution to Construct Fitness Functions in Genetic Programming for Multiclass Object Classification](https://github.com/morinim/vita/wiki/bibliography#13)
///
template<class T>
class gaussian_evaluator : public classification_evaluator<T>
{
public:
  explicit gaussian_evaluator(dataframe &d) : classification_evaluator<T>(d) {}

  fitness_t operator()(const T &) override;
  std::unique_ptr<basic_lambda_f> lambdify(const T &) const override;
};

///
/// Single class evaluator for classification problems.
///
template<class T>
class binary_evaluator : public classification_evaluator<T>
{
public:
  explicit binary_evaluator(dataframe &d) : classification_evaluator<T>(d) {}

  fitness_t operator()(const T &) override;
  std::unique_ptr<basic_lambda_f> lambdify(const T &) const override;
};

#include "kernel/src/evaluator.tcc"
}  // namespace vita

#endif  // include guard
