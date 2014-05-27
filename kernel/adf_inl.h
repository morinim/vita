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

#if !defined(VITA_ADF_INL_H)
#define      VITA_ADF_INL_H

///
/// \param[in] ind individual whose code is used as ADF/ADT.
///
template<class T>
adf_core<T>::adf_core(const T &ind) : code_(ind), id_(adf_count())
{
}

///
/// \param[in] prefix a string identifying adf type.
/// \return a string identifying an ADF/ADT.
///
template<class T>
std::string adf_core<T>::display(const std::string &prefix) const
{
  return prefix + std::to_string(id_);
}

///
/// \return the code (program) of the ADF/ADT.
///
template<class T>
const T &adf_core<T>::code() const
{
  return code_;
}

///
/// \return \c true if the \a object passes the internal consistency check.
///
template<class T>
bool adf_core<T>::debug() const
{
  return code_.eff_size() >= 2;
}

#endif  // Include guard
