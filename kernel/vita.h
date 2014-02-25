/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
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
#include <limits>
#include <memory>

#include "kernel/compatibility_patch.h"

namespace vita
{
#if defined(VITA_NO_LIB)
#  define VITA_INLINE inline
#else
#  define VITA_INLINE
#endif

  /// This is the type used as key for symbol identification.
  typedef unsigned opcode_t;

  /// In an environmnet where a symbol such as + may have many different
  /// meanings, it is useful to specify a "domain of computation" to restrict
  /// attention to specific meanings of interest (e.g. double domain: 1 + 1 = 2;
  /// string domain: "a" + "b" = "ab").
  /// The operations of a domain are defined in files named after the domain
  /// and grouped in the \c primitive/ folder.
  enum class domain_t {d_void = 0, d_bool, d_double, d_int, d_string};

  /// A category provide operations which supplement or supersede those of the
  /// domain but which are restricted to values lying in the (sub)domain by
  /// which is parametrized.
  /// For instance the number 4.0 (in the domain \c d_double) may be present in
  /// two distinct category: 2 (e.g. the category "km/h") and 3 (e.g. the
  /// category "kg").
  /// Categories are the way strong typing GP is implemented in Vita.
  typedef size_t category_t;

  /// Index in the genome. Locus is uniquely identified by a couple of indexes.
  typedef size_t index_t;

  ///
  /// When absolute value of two real numbers is under \a float_epsilon, they
  /// are considered equivalent.
  ///
  const double float_epsilon(0.0001);

  ///
  /// \tparam T a C++ type.
  /// \return the maximum value of type \a T.
  ///
  template<class T> inline T type_max(T)
  {
    return std::numeric_limits<T>::max();
  }

  ///
  /// An implementation of make_unique() as proposed by Herb Sutter in
  /// GotW #102.
  ///
  template<typename T, typename ...Args>
  std::unique_ptr<T> make_unique(Args&& ...args)
  {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

  ///
  /// \return an std::array filled with value \a v.
  ///
  /// This function is used to initialize array in member initialization list
  /// of a constructor (so to be compliant with Effective C++).
  /// To be efficient the compiler need to perform the RVO optimization / copy
  /// elision.
  ///
  /// \see
  /// <http://stackoverflow.com/questions/21993780/fill-stdarray-in-the-member-initialization-list>
  ///
  template<class T, unsigned N> std::array<T, N> make_array(T v)
  {
    std::array<T, N> temp;
    temp.fill(v);
    return temp;
  }

  /// \brief Prefix for debug messages
  const char k_s_debug[] = "[DEBUG]";
  /// \brief Prefix for error messages
  const char k_s_error[] = "[ERROR]";
  /// \brief Prefix for information messages
  /// Information messages regard the status of the program.
  const char k_s_info[] = "[INFO]";
  /// \brief Prefix for warning messages
  const char k_s_warning[] = "[WARNING]";
}  // namespace vita

/// \page page1 VITA Architecture
/// http://code.google.com/p/vita/wiki/Anatomy
/// \page page2 Contributor guidelines
/// http://code.google.com/p/vita/wiki/ContributorGuidelines

#endif  // Include guard
