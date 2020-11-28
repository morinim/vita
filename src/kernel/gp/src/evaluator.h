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
#include "kernel/gp/src/detail/evaluator.h"

namespace vita
{

///
/// An evaluator specialized for symbolic regression / classification problems.
///
/// \tparam T   type of individual
/// \tparam DAT type of the dataset
///
/// This specialization of the evaluator class is "dataset-aware". It's useful
/// to group common factors of more specialized symbolic regression or
/// classification classes.
///
template<class T, class DAT = dataframe>
class src_evaluator : public evaluator<T>
{
public:
  static_assert(detail::is_iterable_v<DAT>);

  explicit src_evaluator(DAT &);

protected:
  DAT *dat_;
};

///
/// An evaluator to minimize the sum of some sort of error.
///
/// \tparam T    type of individual
/// \tparam ERRF the error functor. It returns a measurement of the error that
///              a given program commits in the current training case
///              (`DAT::example`)
/// \tparam DAT  type of the dataset
///
/// This class drive the evolution towards the minimum sum of some sort of
/// error.
///
/// \see mse_evaluator, mae_evaluator, rmae_evaluator, count_evaluator
///
template<class T, class ERRF, class DAT = dataframe>
class sum_of_errors_evaluator : public src_evaluator<T, DAT>
{
public:
  static_assert(std::is_class_v<ERRF>);
  static_assert(detail::is_iterable_v<DAT>);
  static_assert(detail::is_error_functor_v<ERRF, DAT>);

  explicit sum_of_errors_evaluator(DAT &);

  fitness_t operator()(const T &) override;
  fitness_t fast(const T &) override;
  std::unique_ptr<basic_lambda_f> lambdify(const T &) const override;

private:
  fitness_t sum_of_errors_impl(const T &, unsigned);
};

///
/// Mean Absolute Error.
///
/// This functor will drive the evolution towards the minimum sum of
/// absolute errors (\f$\frac{1}{n} \sum_{i=1}^n |target_i - actual_i|\f$).
///
/// There is also a penalty for illegal values (it's a function of the number
/// of illegal values).
///
/// \see mae_evaluator
///
template<class T>
class mae_error_functor
{
public:
  mae_error_functor(const T &);

  double operator()(const dataframe::example &) const;

private:
  basic_reg_lambda_f<T, false> agent_;
};

///
/// Evaluator based on the mean absolute error.
///
/// \see mae_error_functor
///
template<class T, class ERRF = mae_error_functor<T>>
class mae_evaluator : public sum_of_errors_evaluator<T, ERRF>
{
public:
  using sum_of_errors_evaluator<T, ERRF>::sum_of_errors_evaluator;
};

///
/// Mean of Relative Differences.
///
/// This functor will drive the evolution towards the minimum sum of
/// relative differences between target values and actual ones:
///
/// \f[\frac{1}{n} \sum_{i=1}^n \frac{|target_i - actual_i|}{\frac{|target_i| + |actual_i|}{2}}\f]
///
/// This is similar to mae_error_functor but here we sum the *relative* errors.
/// The idea is that the absolute difference of `1` between `6` and `5` is more
/// significant than the same absolute difference between `1000001` and
/// `1000000`.
/// The mathematically precise way to express this notion is to calculate the
/// relative difference.
///
/// \see rmae_evaluator
/// \see https://github.com/morinim/documents/blob/master/math_notes/relative_difference.md
///
template<class T>
class rmae_error_functor
{
public:
  explicit rmae_error_functor(const T &);

  double operator()(const dataframe::example &) const;

private:
  basic_reg_lambda_f<T, false> agent_;
};

///
/// Evaluator based on the mean of relative differences.
///
/// \see rmae_error_functor
///
template<class T, class ERRF = rmae_error_functor<T>>
class rmae_evaluator : public sum_of_errors_evaluator<T, ERRF>
{
public:
  using sum_of_errors_evaluator<T, ERRF>::sum_of_errors_evaluator;
};

///
/// Mean Squared Error.
///
/// This fumctpr will drive the evolution towards the minimum sum of
/// squared errors (\f$\frac{1}{n} \sum_{i=1}^n (target_i - actual_i)^2\f$).
///
/// There is also a penalty for illegal values (it's a function of the number
/// of illegal values).
///
/// \note
/// Real data always have noise (sampling/measurement errors) and noise
/// tends to follow a Gaussian distribution. It can be shown that when we
/// have a bunch of data with errors drawn from such a distribution you are
/// most likely to find the "correct" underlying model if you seek to
/// minimize the sum of squared errors.
///
/// \remark
/// When the dataset contains outliers, the mse_error_functor will heavily
/// weight each of them (this is the result of squaring the outliers).
/// mae_error_functor is less sensitive to the presence of outliers (a
/// desirable property in many applications).
///
/// \see mse_evaluator
///
template<class T>
class mse_error_functor
{
public:
  explicit mse_error_functor(const T &);

  double operator()(const dataframe::example &) const;

private:
  basic_reg_lambda_f<T, false> agent_;
};

///
/// Evaluator based on the mean squared error.
///
/// \see mse_error_functor
///
template<class T, class ERRF = mse_error_functor<T>>
class mse_evaluator : public sum_of_errors_evaluator<T, ERRF>
{
public:
  using sum_of_errors_evaluator<T, ERRF>::sum_of_errors_evaluator;
};

///
/// Number of matches functor.
///
/// This functor will drive the evolution towards the maximum sum of matches
/// (\f$\sum_{i=1}^n target_i == actual_i\f$). Incorrect answers receive the
/// same penalty.
///
/// \see count_evaluator
///
template<class T>
class count_error_functor
{
public:
  explicit count_error_functor(const T &);

  double operator()(const dataframe::example &) const;

private:
  basic_reg_lambda_f<T, false> agent_;
};

///
/// Evaluator based on the number of matches.
///
/// \see count_error_functor
///
template<class T, class ERRF = count_error_functor<T>>
class count_evaluator : public sum_of_errors_evaluator<T, ERRF>
{
public:
  using sum_of_errors_evaluator<T, ERRF>::sum_of_errors_evaluator;
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

#include "kernel/gp/src/evaluator.tcc"
}  // namespace vita

#endif  // include guard
