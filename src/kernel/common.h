/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
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
/// A category provide operations which supplement or supersede those of the
/// domain but which are restricted to values lying in the (sub)domain by
/// which is parametrized.
/// For instance the number 4.0 (in the domain `d_double`) may be present
/// in two distinct category: 2 (e.g. the category "km/h") and 3 (e.g. the
/// category "kg").
/// Categories are the way strong typing GP is enforced in Vita.
using category_t = std::size_t;
constexpr category_t undefined_category = static_cast<category_t>(-1);
using cvect = std::vector<category_t>;  // often used so we define a shortcut

using terminal_param_t = double;

/// A macro to disallow the copy constructor and operator= functions.
/// A good alternative is boost::noncopyable but that gives some false
/// positive with `g++ -Weffc++` so, for now, we keep the macro.
///
/// \see Further details:
/// - <https://stackoverflow.com/q/7823990/3235496>
/// - <https://stackoverflow.com/q/1454407/3235496>
#define DISALLOW_COPY_AND_ASSIGN(Class) \
  Class(const Class &) = delete;        \
  Class &operator=(const Class &) = delete
}  // namespace vita

#endif  // include guard
