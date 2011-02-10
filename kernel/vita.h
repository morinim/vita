/**
 *
 *  \file vita.h
 *
 *  \author EOS di Manlio Morini
 *  \date 2010/12/30
 *
 *  This file is part of VITA
 *
 *
 *  \mainpage VITA v0.9.0
 *
 *  \section note_sec Notes
 *  New versions of this program will be available at http://www.vita.cc
 *
 *  Please reports any sugestions and/or bugs to info@vita.cc
 *
 */
  
#if !defined(VITA_H)
#define      VITA_H

#include <cassert>
#include <climits>
#include <cmath>
#include <boost/cstdint.hpp>

namespace vita
{

#if defined(_MSC_VER)
#  define isnan(x) _isnan(x)
#  define isinf(x) !_finite(x)
#endif

  typedef boost::uint16_t locus_t;
  typedef boost::uint16_t opcode_t;

  typedef unsigned symbol_t;

  inline bool is_bad(double x) { return isinf(x) || isnan(x); }

  ///
  /// When absolute value of two real numbers is under \a float_epsilon, they
  /// are considered equivalent.
  ///
  const double float_epsilon(0.0001);

}  // namespace vita

/// \page page1 VITA Architecture
/// The object oriented architecture of VITA is divided into three pieces:
/// <ol>
///   <li>
///     the Individual \c class, which is the underlying program representation;
///   </li>
///   <li>
///     the Population \c class, which holds a population of Individual(s);
///   </li>
///   <li>
///     the Problem \c class, which holds the objectives, functions and data
///     necessary to define and solve a particular task.
///   </li>
/// </ul>
///

#endif  // VITA_H
