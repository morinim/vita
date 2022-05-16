/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2020-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_VALUE_H)
#define      VITA_VALUE_H

#include <iosfwd>
#include <string>
#include <variant>

namespace vita
{

///
/// In an environment where a symbol such as '+' may have many different
/// meanings, it's useful to specify a "domain of computation" to restrict
/// attention to specific meanings of interest (e.g. double domain:
/// `1 + 1 = 2`; string domain: `"a" + "b" = "ab"`).
///
/// The operations of a domain are defined in files named after the domain
/// and grouped in the `src/primitive/` folder.
///
/// \see category_t
///
enum domain_t {d_void = 0, d_int, d_double, d_string};

using D_VOID   = std::monostate;
using D_INT    =            int;
using D_DOUBLE =         double;
using D_STRING =    std::string;

///
/// A variant containing the data types used by the interpreter for internal
/// calculations / output value and for storing examples.
///
using value_t = std::variant<D_VOID, D_INT, D_DOUBLE, D_STRING>;

///
/// \param[in] v value to be checked
/// \return      `true` if `v` isn't empty
///
inline bool has_value(const value_t &v)
{
  return !std::holds_alternative<std::monostate>(v);
}

std::ostream &operator<<(std::ostream &, const value_t &);

}  // namespace vita

#endif  // include guard
