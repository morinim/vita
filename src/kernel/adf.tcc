/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_ADF_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_ADF_TCC)
#define      VITA_ADF_TCC

///
/// \param[in] ind individual whose code is used as ADF/ADT
///
template<class T>
adf_core<T>::adf_core(const T &ind) : code_(ind), id_(adf_count())
{
}

///
/// \param[in] prefix a string identifying adf type
/// \return           an unique identifier (string) for this ADF/ADT symbol
///
template<class T>
std::string adf_core<T>::name(const std::string &prefix) const
{
  return prefix + std::to_string(id_);
}

///
/// \return the code (program) of the ADF/ADT
///
template<class T>
const T &adf_core<T>::code() const
{
  return code_;
}

///
/// \return `true` if the object passes the internal consistency check
///
template<class T>
bool adf_core<T>::is_valid() const
{
  return code_.active_symbols() >= 2;
}

#endif  // include guard
