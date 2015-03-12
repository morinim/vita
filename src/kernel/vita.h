/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_H)
#define      VITA_H

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

#include "kernel/compatibility_patch.h"

namespace vita
{
#if defined(VITA_NO_LIB)
#  define VITA_INLINE inline
#else
#  define VITA_INLINE
#endif

/// This is the type used as key for symbol identification.
using opcode_t = unsigned;

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

/// \brief Prefix for debug messages
const char k_s_debug[] = "[DEBUG]";
/// \brief Prefix for error messages
const char k_s_error[] = "[ERROR]";
/// \brief Prefix for information messages
/// Information messages regard the status of the program.
const char k_s_info[] = "[INFO]";
/// \brief Prefix for warning messages
const char k_s_warning[] = "[WARNING]";

/// A macro to disallow the copy constructor and operator= functions.
/// A good alternative is boost::noncopyable but that gives some false
/// positive with `g++ -Weffc++` so, for now, we keep the macro.
///
/// \see <http://stackoverflow.com/q/7823990/3235496>
#define DISALLOW_COPY_AND_ASSIGN(Class) \
  Class(const Class &) = delete;        \
  Class &operator=(const Class &) = delete
}  // namespace vita

/// \page page1 VITA Architecture
/// * [Tutorial](https://bitbucket.org/morinim/vita/wiki/quick_start)
/// * [Anatomy of Vita](https://bitbucket.org/morinim/vita/wiki/anatomy)
/// \page page2 Contributor guidelines
/// * [Coding style](https://bitbucket.org/morinim/vita/wiki/coding_style)
/// * [Development cycle](https://bitbucket.org/morinim/vita/wiki/development_cycle)

#endif  // Include guard
