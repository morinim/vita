/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_VALUE_H)
#define      VITA_VALUE_H

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
/// and grouped in the `primitive/` folder.
///
/// \see category_t
///
enum domain_t {d_void = 0, d_int, d_double, d_string};

///
/// A variant containing the data types used by the interpreter for internal
/// calculations / output value and for storing examples.
///
using value_t = std::variant<std::monostate, int, double, std::string>;

static_assert(std::is_same_v<std::monostate,
                             std::variant_alternative_t<d_void, value_t>>);
static_assert(std::is_same_v<int,
                             std::variant_alternative_t<d_int, value_t>>);
static_assert(std::is_same_v<double,
                             std::variant_alternative_t<d_double, value_t>>);
static_assert(std::is_same_v<std::string,
                             std::variant_alternative_t<d_string, value_t>>);

///
/// \param[in] v value to be checked
/// \return      `true` if the `v` isn't empty
///
inline bool has_value(const value_t &v)
{
  return !std::holds_alternative<std::monostate>(v);
}

}  // namespace vita

#endif  // include guard
