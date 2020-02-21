/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
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
/// Calculates the output of a program (individual) given a specific input.
///
/// \param[in] ex a vector of values for the problem's variables
/// \return       the output value of the src_interpreter
///
template<class T>
std::any src_interpreter<T>::run(const std::vector<std::any> &ex)
{
  example_ = &ex;
  return this->run();
}

///
/// Used by the vita::variable class to retrieve the value of a variable.
///
/// \param[in] i the index of a variable
/// \return      the value of the `i`-th variable
///
template<class T>
std::any src_interpreter<T>::fetch_var(unsigned i)
{
  Expects(i < example_->size());
  return (*example_)[i];
}

#endif  // include guard
