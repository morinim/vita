/**
 *
 *  \file vita.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *
 *  \mainpage VITA v0.9.1
 *
 *  \section Introduction
 *  Welcome to the Vita project.
 *
 *  This is the reference guide for the Vita APIs.
 *
 *  \section note_sec Notes
 *  New versions of this program will be available at
 *  http://code.google.com/p/vita/
 *
 *  Please reports any suggestions and/or bugs to:
 *  <dl>
 *    <dt>Forum</dt>
 *    <dd>http://groups.google.com/group/vita-prj</dd>
 *    <dt>Issue tracking system</dt>
 *    <dd>http://code.google.com/p/vita/issues/list</dd>
 *  </dl>
 *
 */

#pragma GCC diagnostic ignored "-Wformat"

#if !defined(VITA_H)
#define      VITA_H

#include <boost/cstdint.hpp>

#include <cassert>
#include <climits>
#include <cmath>
#include <memory>

namespace vita
{
  class symbol;

  /// Just a shortcut.
  typedef std::shared_ptr<symbol> symbol_ptr;

#if defined(_MSC_VER)
#  define ROTL64(x, y)  _rotl64(x, y)
#else
   ///
   /// \param[in] x unsigned 64-bit to be rotated.
   /// \param[in] r number of steps.
   /// \return the value corresponding to rotating the bits of \a x \a r-steps
   ///         to the right (r must be between 1 to 31 inclusive).
   ///
   inline boost::uint64_t rotl64(boost::uint64_t x, boost::uint8_t r)
   {
     return (x << r) | (x >> (64-r));
   }
#  define ROTL64(x, y)  rotl64(x, y)
#endif

  /// This is the type used as key for symbol identification.
  typedef boost::uint16_t opcode_t;

  /// In an environmnet where a symbol such as + may have many different
  /// meanings, it is useful to specify a "domain of computation" to restrict
  /// attention to specific meanings of interest (e.g. double domain: 1 + 1 = 2;
  /// string domain: "a" + "b" = "ab").
  /// The operations of a domain are defined in files named after the domain
  /// and grouped in the \c primitive/ folder.
  enum domain_t {d_void = 0, d_bool, d_double, d_int, d_string};

  /// A category provide operations which supplement or supersede those of the
  /// domain but which are restricted to values lying in the (sub)domain by
  /// which is parametrized.
  /// For instance the number 4.0 (in the domain \c d_double) may be present in
  /// two distinct category: 2 (e.g. the category "km/h") and 3 (e.g. the
  /// category "kg").
  /// Categories are the way strong typing GP is implemented in Vita.
  typedef unsigned category_t;

  /// Index in the genome. Locus is uniquely identified by a couple of indexes.
  typedef unsigned index_t;

  ///
  /// When absolute value of two real numbers is under \a float_epsilon, they
  /// are considered equivalent.
  ///
  const double float_epsilon(0.0001);
}  // namespace vita

/// \page page1 VITA Architecture
/// http://code.google.com/p/vita/wiki/Anatomy

#endif  // VITA_H
