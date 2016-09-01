/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_COMMON_H)
#define      VITA_COMMON_H

// This file must be kept small! Avoid the One Big Header File C/C++
// anti-pattern (and its lack of modularity).
// Just put here:
// - `#include`s that address portability issues or workarounds for broken
//   headers;
// - conditional-compilation macros and instructions that affect the whole
//   codebase;
// - any other definitions that **really** are needed in every compilation
//   unit.
// See also <http://c2.com/cgi/wiki?OneBigHeaderFile>.

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

#include "kernel/compatibility_patch.h"
#include "utility/contracts.h"

namespace vita
{
/// In an environment where a symbol such as '+' may have many different
/// meanings, it's useful to specify a "domain of computation" to restrict
/// attention to specific meanings of interest (e.g. double domain:
/// 1 + 1 = 2; string domain: "a" + "b" = "ab").
/// The operations of a domain are defined in files named after the domain
/// and grouped in the `primitive/` folder.
enum class domain_t {d_void = 0, d_bool, d_double, d_int, d_string};

/// A category provide operations which supplement or supersede those of the
/// domain but which are restricted to values lying in the (sub)domain by
/// which is parametrized.
/// For instance the number 4.0 (in the domain `d_double`) may be present
/// in two distinct category: 2 (e.g. the category "km/h") and 3 (e.g. the
/// category "kg").
/// Categories are the way strong typing GP is implemented in Vita.
using category_t = unsigned;

/// A useful shortcut for a group of categories.
using cvect = std::vector<category_t>;

/// Index in the genome. Locus is uniquely identified by an index and a
/// category.
using index_t = unsigned;

/// A macro to disallow the copy constructor and operator= functions.
/// A good alternative is boost::noncopyable but that gives some false
/// positive with `g++ -Weffc++` so, for now, we keep the macro.
///
/// \see
/// <http://stackoverflow.com/q/7823990/3235496> and
/// <http://stackoverflow.com/q/1454407/3235496> for further details.
#define DISALLOW_COPY_AND_ASSIGN(Class) \
  Class(const Class &) = delete;        \
  Class &operator=(const Class &) = delete
}  // namespace vita

#endif  // include guard
