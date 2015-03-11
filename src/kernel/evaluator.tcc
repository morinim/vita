/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2015 EOS di Manlio Morini.
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
/// \param[in] i an individual to be evaluated.
/// \return the fitness (usually an approximation of) `i`.
///
/// Some evaluators have a faster but approximated version of the standard
/// fitness evaluation method.
///
/// The default implementation calls the standard fitness function.
///
template<class T>
fitness_t evaluator<T>::fast(const T &i)
{
  return operator()(i);
}

///
/// \return the accuracy of a program. A negative value means that accuracy
///         isn't available (`-1.0` is the default value returned).
///
/// Accuracy refers to the number of training examples that are correctly
/// valued/classified as a proportion of the total number of examples in
/// the training set. According to this design, the best accuracy is 1.0
/// (100%), meaning that all the training examples have been correctly
/// recognized.
///
/// \note
/// Accuracy and fitness aren't the same thing.
/// Accuracy can be used to measure fitness but it sometimes hasn't
/// enough "granularity"; also it isn't appropriated for classification
/// tasks with imbalanced learning data (where at least one class is
/// under/over represented relative to others).
///
template<class T>
double evaluator<T>::accuracy(const T &) const
{
  return -1.0;
}

///
/// \return `0`.
///
/// Some evaluators keep additional statistics about the individual seen
/// so far.
///
template<class T>
unsigned evaluator<T>::seen(const T &) const
{
  return 0;
}

///
/// Some evaluators keep a cache / some statistics to improve performances.
/// This method asks to empty the cache / clear the statistics.
///
/// The default implementation is empty.
///
template<class T>
void evaluator<T>::clear(clear_flag)
{
}

///
/// Some evaluators keep a cache to improve performances. This method
/// asks to clear cached informations about an individual.
///
template<class T>
void evaluator<T>::clear(const T &)
{
}

///
/// \return some info about the status / efficiency of the evaluator.
///
/// The default implementation is empty.
///
template<class T>
std::string evaluator<T>::info() const
{
  return std::string();
}

///
/// \return `nullptr`.
///
/// Derived methods create the 'executable' form of an individual.
///
/// The default implementation is an empty method.
///
template<class T>
std::unique_ptr<lambda_f<T>> evaluator<T>::lambdify(const T &) const
{
  return nullptr;
}

///
/// \return a random fitness.
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

#endif  // Include guard
