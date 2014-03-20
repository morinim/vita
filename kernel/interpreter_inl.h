/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_INTERPRETER_INL_H)
#define      VITA_INTERPRETER_INL_H

///
/// \param[in] p a program to be evaluated.
/// \param[in] ctx context in which we calculate the output value of \a p (used
///                for the evaluation of ADF).
///
template<class T>
core_interpreter<T>::core_interpreter(const T &p, core_interpreter<T> *ctx)
  : prg_(p), context_(ctx)
{
}

///
/// \return \c true if the object passes the internal consistency check.
///
template<class T>
bool core_interpreter<T>::debug() const
{
  if (context_ && !context_->debug())
    return false;

  return prg_.debug();
}

#endif  // Include guard
