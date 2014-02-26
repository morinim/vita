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

#include <boost/lexical_cast.hpp>

#include "kernel/any.h"

namespace vita
{
  ///
  /// \param[in] a value that should be converted to \c double.
  /// \return the result of the conversion of \a a in a \c double.
  ///
  /// This function is useful for:
  /// * debugging purpose (otherwise comparison of \c any values is
  ///   complex);
  /// * symbolic regression and classification task (the value returned by
  ///   the interpeter will be used in a "numeric way").
  ///
  template<>
  double to<double>(const any &a)
  {
    // The pointer form of any_cast uses the nullability of pointers (will
    // return a null pointer rather than throw if the cast fails).
    // The alternatives are a.type() == typeid(double)... or try/catch and
    // both seems inferior.
    if (auto *p = any_cast<double>(&a))
      return *p;

    if (auto *p = any_cast<int>(&a))
      return static_cast<double>(*p);

    if (auto *p = any_cast<bool>(&a))
      return static_cast<double>(*p);

    return 0.0;
  }

  ///
  /// \param[in] a value that should be converted to \c std::string.
  /// \return the result of the conversion of \a a in a string.
  ///
  /// This function is useful for debugging purpose (otherwise comparison /
  /// printing of \c any values is complex).
  ///
  template<>
  std::string to<std::string>(const any &a)
  {
    return
      a.type() == typeid(double) ?
      boost::lexical_cast<std::string>(any_cast<double>(a)) :
      a.type() == typeid(int) ?
      boost::lexical_cast<std::string>(any_cast<int>(a)) :
      a.type() == typeid(bool) ?
      boost::lexical_cast<std::string>(any_cast<bool>(a)) :
      any_cast<std::string>(a);
  }
}  // namespace vita
