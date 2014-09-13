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

#if !defined(VITA_PROBLEM_INL_H)
#define      VITA_PROBLEM_INL_H

///
/// A new uninitialized problem.
///
template<class P>
problem<P>::problem()
{
}

///
/// \param[in] p a penalty function.
///
template<class P>
problem<P>::problem(P p) : penalty(p)
{
}

///
/// \param[in] p a penalty function.
/// \return a problem.
///
/// This is the so called object generator idiom. Here it's used because C++11
/// standard doesn't allow for template argument deduction from the parameters
/// passed to the constructor.
///
/// Any reasonable compiler will optimize away the temporary object and this
/// is usually faster than a solution based on \c std::function.
///
/// \see <http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Named_Constructor>
///
template<class P>
problem<P> make_problem(P p)
{
  return problem<P>(p);
}

///
/// \param[in] initialize if \c true initialize the environment with default
///                       values.
///
/// Resets the object.
///
template<class P>
void problem<P>::clear(bool initialize)
{
  env = environment(initialize);
  sset = symbol_set();
}

///
/// \return an access point for the dataset.
///
/// The default value is \c nullptr. Not every problem is dataset-based.
///
template<class P>
vita::data *problem<P>::data()
{
  return nullptr;
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return \c true if the object passes the internal consistency check.
///
template<class P>
bool problem<P>::debug(bool verbose) const
{
  if (!env.debug(verbose, false))
    return false;

  return sset.debug();
}
#endif  // Include guard
