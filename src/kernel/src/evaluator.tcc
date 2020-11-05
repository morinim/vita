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
template<class T, class DS>
src_evaluator<T, DS>::src_evaluator(DS &d) : dat_(&d)
{
}

///
/// \param[in] prg program (individual/team) used for fitness evaluation
/// \return        the fitness (greater is better, max is `0`)
///
template<class T, class DS>
fitness_t sum_of_errors_evaluator<T, DS>::operator()(const T &prg)
{
  Expects(!this->dat_->classes());
  Expects(!this->dat_->empty());

  const basic_reg_lambda_f<T, false> agent(prg);

  fitness_t::value_type err(0.0);
  int illegals(0);

  for (auto &example : *this->dat_)
    err += error(agent, example, &illegals);

  assert(total_nr);

  // Note that we take the average error: this way fast() and operator()
  // outputs can be compared.
  return {-err / static_cast<fitness_t::value_type>(this->dat_->size())};
}

///
/// \param[in] prg program (individual/team) used for fitness evaluation
/// \return        the fitness (greater is better, max is `0`)
///
/// This function is similar to operator()() but will skip 3 out of 4
/// training instances, so it's faster ;-)
///
template<class T, class DS>
fitness_t sum_of_errors_evaluator<T, DS>::fast(const T &prg)
{
  assert(!this->dat_->classes());
  assert(!this->dat_->empty());

  const basic_reg_lambda_f<T, false> agent(prg);

  fitness_t::value_type err(0.0);
  int illegals(0);
  std::size_t counter(0);

  for (auto &example : *this->dat_)
    if (this->dat_->size() <= 20 || (counter++ % 5) == 0)
      err += error(agent, example, &illegals);

  // Note that we take the average error: this way fast() and operator()
  // outputs can be compared.
  return {-err / static_cast<fitness_t::value_type>(this->dat_->size())};
}

///
/// \param[in] prg program(individual/team) to be transformed in a lambda
///                function
/// \return        the lambda function associated with `prg` (`nullptr` in case
///                of errors).
///
template<class T, class DS>
std::unique_ptr<basic_lambda_f> sum_of_errors_evaluator<T, DS>::lambdify(
  const T &prg) const
{
  return std::make_unique<basic_reg_lambda_f<T, true>>(prg);
}

///
/// \param[in] agent        lambda function used for the evaluation of the
///                         current program
/// \param[in] t            the current training case
/// \param[in,out] illegals number of illegals values found evaluating the
///                         current program so far
/// \return                 a measurement of the error of the current program
///                         on the training case `t`. The value returned is in
///                         the `[0;+inf[` range
///
template<class T>
double mae_evaluator<T>::error(const basic_reg_lambda_f<T, false> &agent,
                               dataframe::example &t, int *illegals)
{
  number err;

  if (const auto res = agent(t); has_value(res))
    err = std::fabs(lexical_cast<D_DOUBLE>(res) - label_as<D_DOUBLE>(t));
  else
    err = std::pow(100.0, ++(*illegals));

  if (!issmall(err))
    ++t.difficulty;

  return err;
}

///
/// \param[in] agent lambda function used for the evaluation of the current
///                  program
/// \param[in] t     the current training case
/// \return          a measurement of the error of the current program on the
///                  training case `t`. The value returned is in the `[0;200]`
///                  range
///
template<class T>
double rmae_evaluator<T>::error(const basic_reg_lambda_f<T, false> &agent,
                                dataframe::example &t, int *)
{
  number err;

  if (const auto res = agent(t); has_value(res))
  {
    const auto approx(lexical_cast<D_DOUBLE>(res));
    const auto target(label_as<D_DOUBLE>(t));

    const auto delta(std::fabs(target - approx));

    // Check if the numbers are really close. Needed when comparing numbers
    // near zero.
    if (delta <= 10.0 * std::numeric_limits<number>::min())
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
  else
    err = 200.0;

  if (err > 0.0)
    ++t.difficulty;

  return err;
}

///
/// \param[in] agent        lambda function used for the evaluation of the
///                         current program
/// \param[in] t            the current training case
/// \param[in,out] illegals number of illegals values found evaluating the
///                         current program so far
/// \return                 a measurement of the error of the current program
///                         on the training case `t`
///
template<class T>
double mse_evaluator<T>::error(const basic_reg_lambda_f<T, false> &agent,
                               dataframe::example &t, int *illegals)
{
  number err;

  if (const auto res = agent(t); has_value(res))
  {
    err = lexical_cast<D_DOUBLE>(res) - label_as<D_DOUBLE>(t);
    err *= err;
  }
  else
    err = std::pow(100.0, ++(*illegals));

  if (!issmall(err))
    ++t.difficulty;

  return err;
}

///
/// \param[in] agent lambda function used for the evaluation of the current
///                  program
/// \param[in] t     the current training case
/// \return          a measurement of the error of the current program on the
///                  training case `t`
///
template<class T>
double count_evaluator<T>::error(const basic_reg_lambda_f<T, false> &agent,
                                 dataframe::example &t, int *)
{
  const auto res(agent(t));

  const bool err(!has_value(res) ||
                 !issmall(lexical_cast<D_DOUBLE>(res) - label_as<D_DOUBLE>(t)));

  if (err)
    ++t.difficulty;

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
