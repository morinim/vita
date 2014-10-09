/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_CONSTRAINED_EVALUATOR_INL_H)
#define      VITA_CONSTRAINED_EVALUATOR_INL_H

///
/// \param[in] e pointer to the base evaluator.
/// \param[in] p pointer to a penalty function (as described in "An Efficient
///              Constraint Handling Method for Genetic Algorithms" - Kalyanmoy
///              Deb)
///
template<class T>
constrained_evaluator<T>::constrained_evaluator(
  std::unique_ptr<evaluator<T>> e, penalty_func_t<T> p)
  : eva_(std::move(e)), penalty_(p)
{
  assert(eva_);
  assert(penalty_);
}

///
/// \param[in] prg the program (individual/team) whose fitness we want to know.
/// \return the fitness of \a ind.
///
template<class T>
fitness_t constrained_evaluator<T>::operator()(const T &prg)
{
  return combine(fitness_t{static_cast<fitness_t::value_type>(-penalty_(prg))},
                 (*eva_)(prg));
}

///
/// \param[in] prg the program (individual/team) whose fitness we want to know.
/// \return the an approximation of the fitness of \a i.
///
template<class T>
fitness_t constrained_evaluator<T>::fast(const T &prg)
{
  return combine(fitness_t{static_cast<fitness_t::value_type>(-penalty_(prg))},
                 eva_->fast(prg));
}

///
/// \return the accuracy of \a prg.
///
template<class T>
double constrained_evaluator<T>::accuracy(const T &prg) const
{
  return eva_->accuracy(prg);
}

///
/// \param[in] prg a program (individual/team).
/// \return a pointer to the executable version of \a prg.
///
template<class T>
std::unique_ptr<lambda_f<T>> constrained_evaluator<T>::lambdify(
  const T &prg) const
{
  return eva_->lambdify(prg);
}

#endif  // Include guard
