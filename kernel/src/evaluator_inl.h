/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_EVALUATOR_INL_H)
#define      VITA_SRC_EVALUATOR_INL_H

///
/// \param[in] d dataset that the evaluator will use.
///
template<class T>
src_evaluator<T>::src_evaluator(data &d) : dat_(&d)
{
}

///
/// \param[in] prg program (individual/team) used for fitness evaluation.
/// \return the fitness (greater is better, max is 0).
///
template<class T>
fitness_t sum_of_errors_evaluator<T>::operator()(const T &prg)
{
  assert(!this->dat_->classes());
  assert(this->dat_->begin() != this->dat_->end());

  const basic_reg_lambda_f<T, false> agent(prg);

  fitness_t::value_type err(0.0);
  int illegals(0);

  // We don't use data::size() since it gives the size of the active dataset,
  // *not* the size of the active *slice* in the dataset (so it isn't
  // appropriate with the DSS algorithm).
  unsigned total_nr(0);

  for (auto &example : *this->dat_)
  {
    err += error(agent, example, &illegals);

    ++total_nr;
  }

  assert(total_nr);

  // Note that we take the average error: this way fast() and operator()
  // outputs can be compared.
  return fitness_t(-err / total_nr);
}

///
/// \param[in] prg program (individual/team) used for fitness evaluation.
/// \return the fitness (greater is better, max is 0).
///
/// This function is similar to operator()() but will skip 3 out of 4
/// training instances, so it's faster ;-)
///
template<class T>
fitness_t sum_of_errors_evaluator<T>::fast(const T &prg)
{
  assert(!this->dat_->classes());
  assert(this->dat_->begin() != this->dat_->end());

  const basic_reg_lambda_f<T, false> agent(prg);

  fitness_t::value_type err(0.0);
  int illegals(0);
  unsigned total_nr(0), counter(0);

  for (auto &example : *this->dat_)
    if (this->dat_->size() <= 20 || (counter++ % 5) == 0)
    {
      err += error(agent, example, &illegals);

      ++total_nr;
    }

  assert(total_nr);

  // Note that we take the average error: this way fast() and operator()
  // outputs can be compared.
  return fitness_t(-err / total_nr);
}

///
/// \param[in] prg program (individual/team) used for scoring accuracy.
/// \return the accuracy.
///
template<class T>
double sum_of_errors_evaluator<T>::accuracy(const T &prg) const
{
  assert(!this->dat_->classes());
  assert(this->dat_->begin() != this->dat_->end());

  const auto f(lambdify(prg));

  std::uintmax_t ok(0), total_nr(0);

  for (const auto &example : *this->dat_)
  {
    const any res((*f)(example));
    if (!res.empty() &&
        issmall(to<number>(res) - example.template cast_output<number>()))
      ++ok;

    ++total_nr;
  }

  assert(total_nr);

  return static_cast<double>(ok) / static_cast<double>(total_nr);
}

///
/// \param[in] prg program(individual/team) to be transformed in a lambda
///                function.
/// \return the lambda function associated with \a prg (\c nullptr in case of
///         errors).
///
template<class T>
std::unique_ptr<lambda_f<T>> sum_of_errors_evaluator<T>::lambdify(
  const T &prg) const
{
  return make_unique<reg_lambda_f<T>>(prg);
}

///
/// \param[in] agent lambda function used for the evaluation of the current
///                  program.
/// \param[in] t the current training case.
/// \param[in,out] illegals number of illegals values found evaluating the
///                         current program so far.
/// \return a measurement of the error of the current program on the
///         training case \a t. The value returned is in the [0;+inf[ range.
///
template<class T>
double mae_evaluator<T>::error(const basic_reg_lambda_f<T, false> &agent,
                               data::example &t, int *const illegals)
{
  const any res(agent(t));

  number err;

  if (res.empty())
    err = std::pow(100.0, ++(*illegals));
  else
    err = std::fabs(to<number>(res) - t.cast_output<number>());

  if (!issmall(err))
    ++t.difficulty;

  return err;
}

///
/// \param[in] agent lambda function used for the evaluation of the current
///                  program.
/// \param[in] t the current training case.
/// \return a measurement of the error of the current program on the
///         training case \a t. The value returned is in the [0;200] range.
///
template<class T>
double rmae_evaluator<T>::error(const basic_reg_lambda_f<T, false> &agent,
                                data::example &t, int *const)
{
  const any res(agent(t));

  number err;

  if (res.empty())
    err = 200.0;
  else
  {
    const auto approx(to<number>(res));
    const auto target(t.cast_output<number>());

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

  if (err > 0.0)
    ++t.difficulty;

  return err;
}

///
/// \param[in] agent lambda function used for the evaluation of the current
///                  program.
/// \param[in] t the current training case.
/// \param[in,out] illegals number of illegals values found evaluating the
///                         current program so far.
/// \return a measurement of the error of the current program on the
///         training case \a t.
///
template<class T>
double mse_evaluator<T>::error(const basic_reg_lambda_f<T, false> &agent,
                               data::example &t, int *const illegals)
{
  const any res(agent(t));
  number err;
  if (res.empty())
    err = std::pow(100.0, ++(*illegals));
  else
  {
    err = to<number>(res) - t.cast_output<number>();
    err *= err;
  }

  if (!issmall(err))
    ++t.difficulty;

  return err;
}

///
/// \param[in] agent lambda function used for the evaluation of the current
///                  program.
/// \param[in] t the current training case.
/// \return a measurement of the error of the current program on the
///         training case \a t.
///
template<class T>
double count_evaluator<T>::error(const basic_reg_lambda_f<T, false> &agent,
                                 data::example &t, int *const)
{
  const any res(agent(t));

  const bool err(res.empty() ||
                 !issmall(to<number>(res) - t.cast_output<number>()));

  if (err)
    ++t.difficulty;

  return err ? 1.0 : 0.0;
}

///
/// \param[in] prg program (individual/team) used for scoring accuracy.
/// \return the accuracy.
///
template<class T>
double classification_evaluator<T>::accuracy(const T &prg) const
{
  assert(this->dat_->classes());
  assert(this->dat_->begin() != this->dat_->end());

  const auto f(this->lambdify(prg));

  std::uintmax_t ok(0), total_nr(0);

  for (const auto &example : *this->dat_)
  {
    if (static_cast<class_lambda_f<T> *>(f.get())->tag(example).first ==
        example.template tag())
      ++ok;

    ++total_nr;
  }

  assert(total_nr);

  return static_cast<double>(ok) / static_cast<double>(total_nr);
}

///
/// \param[in] d training data.
/// \param[in] x_slot basic parameter for the Slotted Dynamic Class Boundary
///                   Determination algorithm.
///
template<class T>
dyn_slot_evaluator<T>::dyn_slot_evaluator(data &d, unsigned x_slot)
  : classification_evaluator<T>(d), x_slot_(x_slot)
{
  assert(x_slot_);
}

///
/// \param[in] ind program used for class recognition.
/// \return the fitness (greater is better, max is 0).
///
template<class T>
fitness_t dyn_slot_evaluator<T>::operator()(const T &ind)
{
  basic_dyn_slot_lambda_f<T, false, false> lambda(ind, *this->dat_, x_slot_);

  fitness_t::value_type err(0.0);
  for (auto &example : *this->dat_)
  {
    const auto probable_class(lambda.tag(example).first);

    if (probable_class != example.template tag())
    {
      ++err;
      ++example.template difficulty;
    }
  }

  return fitness_t(-err);

  // The following code is faster but doesn't work for teams and doesn't
  // "cooperate" with DSS.
  //basic_dyn_slot_lambda_f<T, false, false> lambda(ind, *this->dat_, x_slot_);
  //return fitness_t(100.0 * (lambda.training_accuracy() - 1.0));
}

///
/// \param[in] ind individual to be transformed in a lambda function.
/// \return the lambda function associated with \a ind (\c nullptr in case of
///         errors).
///
template<class T>
std::unique_ptr<lambda_f<T>> dyn_slot_evaluator<T>::lambdify(
  const T &ind) const
{
  return make_unique<dyn_slot_lambda_f<T>>(ind, *this->dat_, x_slot_);
}

///
/// \param[in] ind program used for class recognition.
/// \return the fitness (greater is better, max is 0).
///
/// For details about this algorithm see:
/// * "Using Gaussian Distribution to Construct Fitnesss Functions in Genetic
///   Programming for Multiclass Object Classification" - Mengjie Zhang, Will
///   Smart (december 2005).
///
template<class T>
fitness_t gaussian_evaluator<T>::operator()(const T &ind)
{
  assert(ind.debug());
  assert(this->dat_->classes() > 1);

  basic_gaussian_lambda_f<T, false, false> lambda(ind, *this->dat_);

  fitness_t::value_type d(0.0);
  for (auto &example : *this->dat_)
  {
    const auto res(lambda.tag(example));

    if (res.first == example.template tag())
    {
      // Note:
      // * (1.0 - confidence) is the sum of the errors;
      // * (confidence - 1.0) is the opposite (standardized fitness);
      // * (confidence - 1.0) / (dat_->classes() - 1) is the opposite of the
      //   average error;
      // * (1.0 - confidence) is the uncertainty about the right class;
      // * 0.001 is a scaling factor.
      d += (res.second - 1.0) / (this->dat_->classes() - 1);
    }
    else
    {
      // Note:
      // * the maximum single class error is -1.0;
      // * the maximum average class error is -1.0 / dat_->classes();
      // So -1.0 is like to say that we have a complete failure.
      d -= 1.0;

      ++example.template difficulty;
    }
  }

  return fitness_t(d);
}

///
/// \param[in] ind individual to be transformed in a lambda function.
/// \return the lambda function associated with \a ind (\c nullptr in case of
///         errors).
///
template<class T>
std::unique_ptr<lambda_f<T>> gaussian_evaluator<T>::lambdify(
  const T &ind) const
{
  return make_unique<gaussian_lambda_f<T>>(ind, *this->dat_);
}

///
/// \param[in] ind an individual.
/// \return the fitness of individual \a ind (greater is better, max is 0).
///
template<class T>
fitness_t binary_evaluator<T>::operator()(const T &ind)
{
  auto &dataset(*this->dat_);

  assert(dataset.classes() == 2);

  basic_binary_lambda_f<T, false, false> agent(ind, dataset);
  fitness_t::value_type err(0.0);

  for (auto &example : dataset)
    if (example.tag() != agent.tag(example).first)
    {
      ++example.difficulty;
      ++err;

      // err += std::fabs(val);
    }

  return fitness_t(-err);
}

///
/// \param[in] ind individual to be transformed in a lambda function.
/// \return the lambda function associated with \a ind (\c nullptr in case of
///         errors).
///
template<class T>
std::unique_ptr<lambda_f<T>> binary_evaluator<T>::lambdify(const T &ind) const
{
  return make_unique<binary_lambda_f<T>>(ind, *this->dat_);
}

#endif  // Include guard
