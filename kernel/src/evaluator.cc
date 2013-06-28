/**
 *
 *  \file src/evaluator.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "kernel/src/evaluator.h"
#include "kernel/individual.h"
#include "kernel/src/interpreter.h"

namespace vita
{
  ///
  /// \param[in] d data that the evaluator will use.
  ///
  src_evaluator::src_evaluator(data &d) : dat_(&d) {}

  ///
  /// \param[in] ind program used for fitness evaluation.
  /// \return the fitness (greater is better, max is 0).
  ///
  fitness_t sum_of_errors_evaluator::operator()(const individual &ind)
  {
    assert(!dat_->classes());
    assert(dat_->cbegin() != dat_->cend());

    src_interpreter agent(ind);

    fitness_t::base_t err(0.0);
    int illegals(0);

    // We don't use data::size() since it gives the size of the active dataset,
    // *not* the size of the active *slice* in the dataset (so it isn't
    // appropriate with the DSS algorithm).
    unsigned total_nr(0);

    for (auto &example : *dat_)
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
  /// \param[in] ind program used for fitness evaluation.
  /// \return the fitness (greater is better, max is 0).
  ///
  /// This function is similar to operator()() but will skip 3 out of 4
  /// training instances, so it's faster ;-)
  ///
  fitness_t sum_of_errors_evaluator::fast(const individual &ind)
  {
    assert(!dat_->classes());
    assert(dat_->cbegin() != dat_->cend());

    src_interpreter agent(ind);

    fitness_t::base_t err(0.0);
    int illegals(0);
    unsigned total_nr(0), counter(0);

    for (auto &example : *dat_)
      if (dat_->size() <= 20 || (counter++ % 5) == 0)
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
  /// \param[in] ind program used for scoring accuracy.
  /// \return the accuracy.
  ///
  double sum_of_errors_evaluator::accuracy(const individual &ind) const
  {
    assert(!dat_->classes());
    assert(dat_->cbegin() != dat_->cend());

    std::unique_ptr<lambda_f> f(lambdify(ind));

    std::uintmax_t ok(0), total_nr(0);

    for (const auto &example : *dat_)
    {
      const any res((*f)(example));
      if (!res.empty() &&
          std::fabs(interpreter::to_double(res) -
                    data::cast<double>(example.output)) <= float_epsilon)
        ++ok;

      ++total_nr;
    }

    assert(total_nr);

    return static_cast<double>(ok) / static_cast<double>(total_nr);
  }

  ///
  /// \param[in] ind individual to be transformed in a lambda function.
  /// \return the lambda function associated with \a ind (\c nullptr in case of
  ///         errors).
  ///
  std::unique_ptr<lambda_f> sum_of_errors_evaluator::lambdify(
    const individual &ind) const
  {
    return make_unique<lambda_f>(ind);
  }

  ///
  /// \param[in] agent interpreter used for the evaluation of the current
  ///                  individual. Note that this isn't a constant reference
  ///                  because the internal state of agent changes during
  ///                  evaluation; anyway this is an input-only parameter.
  /// \param[in] t the current training case.
  /// \param[in,out] illegals number of illegals values found evaluating the
  ///                         current individual so far.
  /// \return a measurement of the error of the current individual on the
  ///         training case \a t. The value returned is in the [0;+inf[ range.
  ///
  double mae_evaluator::error(src_interpreter &agent, data::example &t,
                              int *const illegals)
  {
    const any res(agent.run(t));

    double err;

    if (res.empty())
      err = std::pow(100.0, ++(*illegals));
    else
      err = std::fabs(interpreter::to_double(res) -
                      data::cast<double>(t.output));

    if (err > float_epsilon)
      ++t.difficulty;

    return err;
  }

  ///
  /// \param[in] agent interpreter used for the evaluation of the current
  ///                  individual. Note that this isn't a constant reference
  ///                  because the internal state of agent changes during
  ///                  evaluation; anyway this is an input-only parameter.
  /// \param[in] t the current training case.
  /// \return a measurement of the error of the current individual on the
  ///         training case \a t. The value returned is in the [0;200] range.
  ///
  double mrae_evaluator::error(src_interpreter &agent, data::example &t,
                               int *const)
  {
    const any res(agent.run(t));

    double err;

    if (res.empty())
      err = 200.0;
    else
    {
      const double approx(interpreter::to_double(res));
      const double target(data::cast<double>(t.output));

      const double delta(std::fabs(target - approx));

      // Check if the numbers are really close. Needed when comparing numbers
      // near zero.
      if (delta <= 10.0 * std::numeric_limits<double>::min())
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
  /// \param[in] agent interpreter used for the evaluation of the current
  ///                  individual. Note that this isn't a constant reference
  ///                  because the internal state of agent changes during
  ///                  evaluation; anyway this is an input-only parameter.
  /// \param[in] t the current training case.
  /// \param[in,out] illegals number of illegals values found evaluating the
  ///                         current individual so far.
  /// \return a measurement of the error of the current individual on the
  ///         training case \a t.
  ///
  double mse_evaluator::error(src_interpreter &agent, data::example &t,
                              int *const illegals)
  {
    const any res(agent.run(t));
    double err;
    if (res.empty())
      err = std::pow(100.0, ++(*illegals));
    else
    {
      err = interpreter::to_double(res) - data::cast<double>(t.output);
      err *= err;
    }

    if (err > float_epsilon)
      ++t.difficulty;

    return err;
  }

  ///
  /// \param[in] agent interpreter used for the evaluation of the current
  ///                  individual. Note that this isn't a constant reference
  ///                  because the internal state of agent changes during
  ///                  evaluation; anyway this is an input-only parameter.
  /// \param[in] t the current training case.
  /// \return a measurement of the error of the current individual on the
  ///         training case \a t.
  ///
  double count_evaluator::error(src_interpreter &agent, data::example &t,
                                int *const)
  {
    const any res(agent.run(t));

    const bool err(res.empty() ||
                   std::fabs(interpreter::to_double(res) -
                             data::cast<double>(t.output)) > float_epsilon);

    if (err)
      ++t.difficulty;

    return err ? 1.0 : 0.0;
  }

  ///
  /// \param[in] ind program used for scoring accuracy.
  /// \return the accuracy.
  ///
  double classification_evaluator::accuracy(const individual &ind) const
  {
    assert(dat_->classes());
    assert(dat_->cbegin() != dat_->cend());

    std::unique_ptr<lambda_f> f(lambdify(ind));

    std::uintmax_t ok(0), total_nr(0);

    for (const auto &example : *dat_)
    {
      const any res((*f)(example));
      if (any_cast<size_t>(res) == example.label())
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
  dyn_slot_evaluator::dyn_slot_evaluator(data &d, size_t x_slot)
    : classification_evaluator(d), x_slot_(x_slot)
  {
    assert(x_slot);
  }

  ///
  /// \param[in] ind program used for class recognition.
  /// \return the fitness (greater is better, max is 0).
  ///
  /// \todo
  /// To date we haven't an efficient way to calculate DSS example difficulty
  /// in combination with Dynamic Slot Algorithm. We skip this calculation,
  /// so DSS isn't working at full full capacity (it considers only example
  /// "age").
  ///
  fitness_t dyn_slot_evaluator::operator()(const individual &ind)
  {
    assert(ind.debug());
    assert(dat_->classes() > 1);

    engine_ = dyn_slot_engine(ind, *dat_, x_slot_);

    fitness_t::base_t err(0.0);
    for (size_t i(0); i < engine_.slot_matrix.rows(); ++i)
      for (size_t j(0); j < engine_.slot_matrix.cols(); ++j)
        if (j != engine_.slot_class[i])
          err += engine_.slot_matrix(i, j);

    assert(engine_.dataset_size >= err);

    return fitness_t(-err);
  }

  ///
  /// \param[in] ind individual to be transformed in a lambda function.
  /// \return the lambda function associated with \a ind (\c nullptr in case of
  ///         errors).
  ///
  std::unique_ptr<lambda_f> dyn_slot_evaluator::lambdify(
    const individual &ind) const
  {
    return make_unique<dyn_slot_lambda_f>(ind, *dat_, x_slot_);
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
  fitness_t gaussian_evaluator::operator()(const individual &ind)
  {
    assert(ind.debug());
    assert(dat_->classes() > 1);
    gaussian_engine engine_(ind, *dat_);

    fitness_t::base_t d(0.0);
    for (auto &example : *dat_)
    {
      double confidence, sum;
      const size_t probable_class(engine_.class_label(ind, example,
                                                      &confidence, &sum));

      if (probable_class == example.label())
      {
        // Note:
        // * (sum - confidence) is the sum of the errors;
        // * (confidence - sum) is the opposite (standardized fitness);
        // * (confidence - sum) / (dat_->classes() - 1) is the opposite of the
        //   average error;
        // * (1.0 - confidence) is the uncertainty about the right class;
        // * 0.001 is a scaling factor.
        d += (confidence - sum) / (dat_->classes() - 1) -
             0.001 * (1.0 - confidence);
      }
      else
      {
        // Note:
        // * the maximum single class error is -1.0;
        // * the maximum average class error is -1.0 / dat_->classes();
        // So -1.0 is like to say that we have a complete failure.
        d -= 1.0;

        ++example.difficulty;
      }
    }

    return fitness_t(d);
  }

  ///
  /// \param[in] ind individual to be transformed in a lambda function.
  /// \return the lambda function associated with \a ind (\c nullptr in case of
  ///         errors).
  ///
  std::unique_ptr<lambda_f> gaussian_evaluator::lambdify(
    const individual &ind) const
  {
    return make_unique<gaussian_lambda_f>(ind, *dat_);
  }

  /*
  ///
  /// \param[in] ind an individual.
  /// \param[in] label
  /// \return the fitness of individual \a ind (greater is better, max is 0).
  ///
  template<class T>
  fitness_t problem::binary_fitness(const vita::individual &ind,
                                    unsigned label) const
  {
    assert(dat_.classes() == 2);

    src_interpreter agent(ind);

    fitness_t::base_t err(0.0);
    int illegals(0);

    for (auto &example : *dat_)
    {
      for (size_t i(0); i < vars_.size(); ++i)
        vars_[i]->val = example->input[i];

      const any res(agent.run());

      if (res.empty())
        err += std::pow(100.0, ++illegals);
      else
      {
        const T val(any_cast<T>(res));
        if ((example->label() == label && val < 0.0) ||
            (example->label() != label && val >= 0.0))
          err += std::fabs(val);
      }
    }

    return -err;
  }
  */
}  // namespace vita
