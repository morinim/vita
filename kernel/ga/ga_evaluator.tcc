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

#if !defined(VITA_GA_EVALUATOR_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_GA_EVALUATOR_TCC)
#define      VITA_GA_EVALUATOR_TCC

///
/// \param[in] f a pointer to an objective function.
///
/// GP-evaluators use datasets, GA-evaluators need functions to be maximized.
///
template<class T, class F>
ga_evaluator<T, F>::ga_evaluator(F f) : f_(f)
{
  assert(f_);
}

///
/// \param[in] f objective function.
/// \return an evaluator based on \a f.
///
/// This is the so called object generator idiom. Here it's used because C++11
/// standard doesn't allow for template argument deduction from the parameters
/// passed to the constructor.
///
/// Any reasonable compiler will optimize away the temporary object and this
/// is usually faster than a solution based on \c std::function.
///
/// \see
/// * <http://stackoverflow.com/q/984394/3235496>
/// * <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3602.html>
///
template<class T, class F>
ga_evaluator<T, F> make_ga_evaluator(F f)
{
  return ga_evaluator<T, F>(f);
}

///
/// \return the fitness of the individual (range [-1000;0]).
///
template<class T, class F>
fitness_t ga_evaluator<T, F>::operator()(const T &ind)
{
  const auto f_v(f_(ind));

  if (std::isfinite(f_v))
    //return {f_v};
    return {1000.0 * (std::atan(f_v) / 3.14159265358979323846 - 0.5)};
    //return {-std::exp(-f_v / 1000.0)};

  return fitness_t();
}
#endif  // Include guard