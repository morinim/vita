/**
 *
 *  \file vita.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *
 *  \mainpage VITA v0.9.1
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
  typedef std::shared_ptr<symbol> symbol_ptr;

#if defined(_MSC_VER)
#  define isnan(x)      _isnan(x)
#  define isinf(x)      !_finite(x)
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

  typedef boost::uint16_t locus_t;
  typedef boost::uint16_t opcode_t;

  typedef unsigned category_t;

  inline bool is_bad(double x) { return isinf(x) || isnan(x); }

  ///
  /// When absolute value of two real numbers is under \a float_epsilon, they
  /// are considered equivalent.
  ///
  const double float_epsilon(0.0001);
}  // namespace vita

/// \page page1 VITA Architecture
/// http://code.google.com/p/vita/wiki/Anatomy

#endif  // VITA_H
