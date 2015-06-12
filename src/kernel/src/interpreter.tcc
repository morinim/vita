/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_INTERPRETER_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_SRC_INTERPRETER_TCC)
#define      VITA_SRC_INTERPRETER_TCC

///
/// \param[in] ex a vector of values for the problem's variables.
/// \return the output value of the src_interpreter.
///
/// Calculates the output of a program (individual) given the `ex` training
/// example.
///
template<class T>
any src_interpreter<T>::run(const std::vector<any> &ex)
{
  example_ = &ex;
  return this->run();
}

///
/// \param[in] i the index of a variable.
/// \return the value of the i-th variable.
///
/// This is used by the vita::variable class to retrieve the value of a
/// variable.
///
template<class T>
any src_interpreter<T>::fetch_var(unsigned i)
{
  assert(i < example_->size());
  return (*example_)[i];
}

#endif  // Include guard
