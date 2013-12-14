/**
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(INTERPRETER_INL_H)
#define      INTERPRETER_INL_H

///
/// \param[in] p a program to be evaluated.
/// \param[in] ctx context in which we calculate the output value of \a p (used
///                for the evaluation of ADF).
///
template<class T>
basic_interpreter<T>::basic_interpreter(const T &p, basic_interpreter<T> *ctx)
  : prg_(p), context_(ctx)
{
}

///
/// \param[in] t a team to be evaluated.
/// \param[in] ctx context in which we calculate the output value of \a t (used
///                for the evaluation of ADF).
///
template<class T>
interpreter<team<T>>::interpreter(const team<T> &t, interpreter<team<T>> *ctx)
  : basic_interpreter<T>(t, ctx)
{
}

///
/// \return the output of the team.
///
template<class T>
any interpreter<team<T>>::run()
{
  const auto size(this->prg_.size());
  std::vector<any> result(size);

  for (auto j(decltype(size){0}); j < size; ++j)
  {
    interpreter<T> i(this->prj_[j], &this->context_[j]);
    result.push_back(i.run());
  }

  return any(result);
}

///
/// \return \c true if the object passes the internal consistency check.
///
template<class T>
bool interpreter<team<T>>::debug() const
{
  for (const auto &i : this->prg_)
    if (!i.debug())
      return false;

  return true;
}

#endif  // INTERPRETER_INL_H
