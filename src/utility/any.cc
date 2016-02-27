/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <string>

#include "utility/any.h"

namespace vita
{
  ///
  /// \param[in] a value that should be converted to `double`.
  /// \return the result of the conversion of \a a in a `double`.
  ///
  /// This function is useful for:
  /// * debugging purpose (otherwise comparison of `any` values is complex);
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
  /// \param[in] a value that should be converted to `std::string`.
  /// \return the result of the conversion of \a a in a string.
  ///
  /// This function is useful for debugging purpose (otherwise comparison /
  /// printing of `any` values is complex).
  ///
  template<>
  std::string to<std::string>(const any &a)
  {
    if (auto *p = any_cast<double>(&a))
      return std::to_string(*p);

    if (auto *p = any_cast<int>(&a))
      return std::to_string(*p);

    if (auto *p = any_cast<bool>(&a))
      return std::to_string(*p);

    return any_cast<std::string>(a);
  }
}  // namespace vita
