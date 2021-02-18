/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2021 EOS di Manlio Morini.
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
/// \param[in] in input stream
/// \return       `true` if the object loaded correctly
///
/// \note
/// If the load operation isn't successful the current object isn't changed.
///
template<class T>
bool evaluator<T>::load(std::istream &)
{
  return true;
}

///
/// \param[out] out output stream
/// \return         `true` if the object was saved correctly
///
template<class T>
bool evaluator<T>::save(std::ostream &) const
{
  return true;
}

///
/// Derived methods create the 'executable' form of an individual.
///
/// \return `nullptr`
///
/// The default implementation is an empty method.
///
template<class T>
std::unique_ptr<basic_lambda_f> evaluator<T>::lambdify(const T &) const
{
  return nullptr;
}

template<class T>
test_evaluator<T>::test_evaluator(test_evaluator_type et) : buffer_(), et_(et)
{
}

///
/// \param[in] prg a program (individual/team)
/// \return        fitness value for individual `prg`
///
/// Depending on the type of test_evaluator returns:
/// - a random, time-invariant fitness value for `prg`;
/// - a fixed, time-invariant fitness value for every individual of the
///   population;
/// - a distinct, time-invariant fitness value for each `prg`.
///
template<class T>
fitness_t test_evaluator<T>::operator()(const T &prg)
{
  if (et_ == test_evaluator_type::fixed)
    return {static_cast<fitness_t::value_type>(0)};

  auto it(std::find(buffer_.begin(), buffer_.end(), prg));
  if (et_ != test_evaluator_type::fixed && it == buffer_.end())
  {
    buffer_.push_back(prg);
    it = std::prev(buffer_.end());
  }

  const auto dist(std::distance(buffer_.begin(), it));

  if (et_ == test_evaluator_type::distinct)
    return {static_cast<fitness_t::value_type>(dist)};

  assert(et_ == test_evaluator_type::random);
  static random::engine_t e;
  e.seed(dist);
  return {static_cast<fitness_t::value_type>(e())};
}

#endif  // include guard
