/**
 *
 *  \file src/interpreter_inl.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_INTERPRETER_INL_H)
#define      SRC_INTERPRETER_INL_H

///
/// \param[in] ex a vector of values for the problem's variables.
/// \return the output value of the src_interpreter.
///
/// Calculates the output of a program (individual) given the \a ex training
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

#endif  // SRC_INTERPRETER_INL_H
