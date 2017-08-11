/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVALUATOR_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_EVALUATOR_TCC)
#define      VITA_EVALUATOR_TCC

///
/// An approximate, faster version of the standard evaluator.
///
/// \param[in] i an individual to be evaluated
/// \return      the fitness (usually an approximation of) `i`
///
/// Some evaluators have a faster but approximated version of the standard
/// fitness evaluation method.
///
/// \note Default implementation calls the standard fitness function.
///
template<class T>
fitness_t evaluator<T>::fast(const T &i)
{
  return operator()(i);
}

///
/// Some evaluators keep additional statistics about the individual seen
/// so far.
///
/// \return `0`
///
template<class T>
unsigned evaluator<T>::seen(const T &) const
{
  return 0;
}

///
/// Empties the cache / clear the statistics.
///
/// Some evaluators keep a cache / some statistics to improve performances.
///
/// \note Default implementation is empty.
///
template<class T>
void evaluator<T>::clear(clear_flag)
{
}

///
/// Clears cached informations about a specific individual.
///
/// Some evaluators keep a cache to improve performances.
///
template<class T>
void evaluator<T>::clear(const T &)
{
}

///
/// \return some info about the status / efficiency of the evaluator
///
/// The default implementation is empty.
///
template<class T>
std::string evaluator<T>::info() const
{
  return std::string();
}

///
/// Derived methods create the 'executable' form of an individual.
///
/// \return `nullptr`
///
/// The default implementation is an empty method.
///
template<class T>
std::unique_ptr<lambda_f<T>> evaluator<T>::lambdify(const T &) const
{
  return nullptr;
}

///
/// \return a random fitness
///
template<class T>
fitness_t random_evaluator<T>::operator()(const T &)
{
  const double sup(16000.0);

  fitness_t f;
  for (unsigned i(0); i < f.size(); ++i)
    f[i] = random::sup(sup);

  return f;
}

///
/// \param[in] prg a program (individual/team)
/// \return        a unique, time-invariant, unspecified fitness value for
///                individual `prg`
///
template<class T>
fitness_t test_evaluator<T>::operator()(const T &prg)
{
  auto it(std::find(buffer_.begin(), buffer_.end(), prg));

  if (it == buffer_.end())
  {
    buffer_.push_back(prg);
    it = std::prev(buffer_.end());
  }

  return {static_cast<fitness_t::value_type>(std::distance(buffer_.begin(),
                                                           it))};
}

#endif  // include guard
