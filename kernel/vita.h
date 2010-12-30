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

  inline bool is_bad(double x) { return isinf(x) || isnan(x); };

  /**
   * float_epsilon
   * When absolute value of two real numbers is under float_epsilon, they are
   * considered equivalent.
   */
  const double float_epsilon(0.0001);

}  // namespace vita

#endif  // VITA_H
