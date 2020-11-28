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
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_SRC_EVALUATOR_TCC)
#define      VITA_SRC_EVALUATOR_TCC

///
/// \param[in] d dataset that the evaluator will use
///
template<class T, class DAT>
src_evaluator<T, DAT>::src_evaluator(DAT &d) : dat_(&d)
{
}

///
/// \param[in] d the training dataset
///
template<class T, class ERRF, class DAT>
sum_of_errors_evaluator<T, ERRF, DAT>::sum_of_errors_evaluator(DAT &d)
  : src_evaluator<T, DAT>(d)
{
}

///
/// Sums the error reported by the error functor over a training set.
///
/// \param[in] prg  program (individual/team) used for fitness evaluation
/// \param[in] step consider just `1` example every `step`
/// \return         the fitness (greater is better, max is `0`)
///
template<class T, class ERRF, class DAT>
fitness_t sum_of_errors_evaluator<T, ERRF, DAT>::sum_of_errors_impl(
  const T &prg, unsigned step)
{
  Expects(this->dat_->begin() != this->dat_->end());
  Expects(!detail::classes(this->dat_));

  const ERRF err_fctr(prg);

  double average_error(0.0), n(0.0);
  for (auto it(std::begin(*this->dat_));
       std::distance(it, std::end(*this->dat_)) >= step;
       std::advance(it, step))
  {
    const auto err(err_fctr(*it));

    // User specified examples could not support difficulty.
    if constexpr (detail::has_difficulty_v<DAT>)
      if (!issmall(err))
        ++it->difficulty;

    average_error += (err - average_error) / ++n;
  }

  // Note that we take the average error: this way fast() and operator()
  // outputs can be compared.
  return {static_cast<fitness_t::value_type>(-average_error)};
}

///
/// \param[in] prg program (individual/team) used for fitness evaluation
/// \return        the fitness (greater is better, max is `0`)
///
template<class T, class ERRF, class DAT>
fitness_t sum_of_errors_evaluator<T, ERRF, DAT>::operator()(const T &prg)
{
  return sum_of_errors_impl(prg, 1);
}

///
/// \param[in] prg program (individual/team) used for fitness evaluation
/// \return        the fitness (greater is better, max is `0`)
///
/// This function is similar to operator()() but will skip 4 out of 5
/// training instances, so it's faster.
///
template<class T, class ERRF, class DAT>
fitness_t sum_of_errors_evaluator<T, ERRF, DAT>::fast(const T &prg)
{
  Expects(std::distance(this->dat_->begin(), this->dat_->end()) >= 100);
  return sum_of_errors_impl(prg, 5);
}

///
/// \param[in] prg program(individual/team) to be transformed in a lambda
///                function
/// \return        the lambda function associated with `prg` (`nullptr` in case
///                of errors).
///
template<class T, class LAMBDA, class DAT>
std::unique_ptr<basic_lambda_f>
sum_of_errors_evaluator<T, LAMBDA, DAT>::lambdify(const T &prg) const
{
  return std::make_unique<basic_reg_lambda_f<T, true>>(prg);
}

///
/// Sets up the environment for error measurement.
///
/// \param[in] prg the program to be measured
///
template<class T>
mae_error_functor<T>::mae_error_functor(const T &prg) : agent_(prg)
{
}

///
/// \param[in] example current training case
/// \return            a measurement of the error of the model/program on the
///                    given training case (value in the `[0;+inf[` range)
///
template<class T>
double mae_error_functor<T>::operator()(const dataframe::example &example) const
{
  if (const auto model_value = agent_(example); has_value(model_value))
    return std::fabs(lexical_cast<D_DOUBLE>(model_value)
                     - label_as<D_DOUBLE>(example));

  return std::numeric_limits<double>::max() / 100.0;
}

///
/// Sets up the environment for error measurement.
///
/// \param[in] prg the program to be measured
///
template<class T>
rmae_error_functor<T>::rmae_error_functor(const T &prg) : agent_(prg)
{
}

///
/// \param[in] example current training case
/// \return            measurement of the error of the model/program on the
///                    current training case. The value returned is in the
///                    `[0;200]` range
///
template<class T>
double rmae_error_functor<T>::operator()(
  const dataframe::example &example) const
{
  double err(200.0);

  if (const auto model_value = agent_(example); has_value(model_value))
  {
    const auto approx(lexical_cast<D_DOUBLE>(model_value));
    const auto target(label_as<D_DOUBLE>(example));

    const auto delta(std::fabs(target - approx));

    // Check if the numbers are really close. Needed when comparing numbers
    // near zero.
    if (delta <= 10.0 * std::numeric_limits<D_DOUBLE>::min())
      err = 0.0;
    else
      err = 200.0 * delta / (std::fabs(approx) + std::fabs(target));
    // Some alternatives for the error:
    // * delta / std::max(approx, target)
    // * delta / std::fabs(target)
    //
    // The chosen formula seems numerically more stable and gives a result
    // in a limited range of values.
  }

  return err;
}

///
/// Sets up the environment for error measurement.
///
/// \param[in] prg the program to be measured
///
template<class T>
mse_error_functor<T>::mse_error_functor(const T &prg) : agent_(prg)
{
}

///
/// \param[in] example current training case
/// \return            a measurement of the error of the model/program on the
///                    the current training case. The value returned is in the
///                    `[0;+inf[` range
///
template<class T>
double mse_error_functor<T>::operator()(const dataframe::example &example) const
{
  if (const auto model_value = agent_(example); has_value(model_value))
  {
    const double err(lexical_cast<D_DOUBLE>(model_value)
                     - label_as<D_DOUBLE>(example));
    return err * err;
  }

  return std::numeric_limits<double>::max() / 100.0;
}

///
/// Sets up the environment for error measurement.
///
/// \param[in] prg the program to be measured
///
template<class T>
count_error_functor<T>::count_error_functor(const T &prg) : agent_(prg)
{
}

///
/// \param[in] example current training case
/// \return            a measurement of the error of the model/program on the
///                    current training case. The value returned is in the
///                    `[0;+inf[` range
///
template<class T>
double count_error_functor<T>::operator()(
  const dataframe::example &example) const
{
  const auto model_value(agent_(example));

  const bool err(!has_value(model_value)
                 || !issmall(lexical_cast<D_DOUBLE>(model_value)
                             - label_as<D_DOUBLE>(example)));

  return err ? 1.0 : 0.0;
}

///
/// \param[in] p      current dataset
/// \param[in] x_slot basic parameter for the Slotted Dynamic Class Boundary
///                   Determination algorithm
///
template<class T>
dyn_slot_evaluator<T>::dyn_slot_evaluator(dataframe &d, unsigned x_slot)
  : classification_evaluator<T>(d), x_slot_(x_slot)
{
  assert(x_slot_);
}

///
/// \param[in] ind program used for class recognition
/// \return        the fitness (greater is better, max is `0`)
///
template<class T>
fitness_t dyn_slot_evaluator<T>::operator()(const T &ind)
{
  basic_dyn_slot_lambda_f<T, false, false> lambda(ind, *this->dat_, x_slot_);

  fitness_t::value_type err(0.0);
  for (auto &example : *this->dat_)
    if (lambda.tag(example).label != label(example))
    {
      ++err;
      ++example.difficulty;
    }

  return {-err};

  // The following code is faster but doesn't work for teams and doesn't
  // "cooperate" with DSS.
  //
  // basic_dyn_slot_lambda_f<T,false,false> lambda(ind, *this->dat_, x_slot_);
  // return {100.0 * (lambda.training_accuracy() - 1.0)};
}

///
/// \param[in] ind individual to be transformed in a lambda function
/// \return        the lambda function associated with `ind` (`nullptr` in case
///                of errors)
///
template<class T>
std::unique_ptr<basic_lambda_f> dyn_slot_evaluator<T>::lambdify(
  const T &ind) const
{
  return std::make_unique<dyn_slot_lambda_f<T>>(ind, *this->dat_, x_slot_);
}

///
/// \param[in] ind program used for class recognition
/// \return        the fitness (greater is better, max is `0`)
///
/// For details about this algorithm see:
/// * "Using Gaussian Distribution to Construct Fitnesss Functions in Genetic
///   Programming for Multiclass Object Classification" - Mengjie Zhang, Will
///   Smart (december 2005).
///
template<class T>
fitness_t gaussian_evaluator<T>::operator()(const T &ind)
{
  assert(this->dat_->classes() >= 2);

  basic_gaussian_lambda_f<T, false, false> lambda(ind, *this->dat_);

  fitness_t::value_type d(0.0);
  for (auto &example : *this->dat_)
    if (const auto res = lambda.tag(example); res.label == label(example))
    {
      const auto scale(static_cast<fitness_t::value_type>(this->dat_->classes()
                                                          - 1));
      // Note:
      // * `(1.0 - res.sureness)` is the sum of the errors;
      // * `(res.sureness - 1.0)` is the opposite (standardized fitness);
      // * `(res.sureness - 1.0) / scale` is the opposite of the average error.
      d += (res.sureness - 1.0) / scale;
    }
    else
    {
      // Note:
      // * the maximum single class error is 1.0;
      // * the maximum average class error is `1.0 / dat_->classes()`;
      // So -1.0 is like to say that we have a complete failure.
      d -= 1.0;

      ++example.difficulty;
    }

  return {d};
}

///
/// \param[in] ind individual to be transformed in a lambda function
/// \return        the lambda function associated with `ind` (`nullptr` in case
///                of errors)
///
template<class T>
std::unique_ptr<basic_lambda_f> gaussian_evaluator<T>::lambdify(
  const T &ind) const
{
  return std::make_unique<gaussian_lambda_f<T>>(ind, *this->dat_);
}

///
/// \param[in] ind an individual
/// \return        the fitness of `ind` (greater is better, max is `0`)
///
template<class T>
fitness_t binary_evaluator<T>::operator()(const T &ind)
{
  Expects(this->dat_->classes() == 2);

  basic_binary_lambda_f<T, false, false> agent(ind, *this->dat_);
  fitness_t::value_type err(0.0);

  for (auto &example : *this->dat_)
    if (label(example) != agent.tag(example).label)
    {
      ++example.difficulty;
      ++err;

      // err += std::fabs(val);
    }

  return {-err};
}

///
/// \param[in] ind individual to be transformed in a lambda function
/// \return        the lambda function associated with `ind` (`nullptr` in case
///                of errors)
///
template<class T>
std::unique_ptr<basic_lambda_f> binary_evaluator<T>::lambdify(
  const T &ind) const
{
  return std::make_unique<binary_lambda_f<T>>(ind, *this->dat_);
}

#endif  // include guard
