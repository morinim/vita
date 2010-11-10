/**
 *
 *  \file vita.h
 *
 *  \author EOS di Manlio Morini
 *  \version 0.9.2
 *  \date 2010/06/11
 *
 *  This file is part of VITA
 *
 *  New versions of this program will be available at http://vita.ws
 *
 *  Please reports any sugestions and/or bugs to info@vita.ws
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
   * gene_args
   * Maximum number of arguments for a vita function.
   */
  const unsigned gene_args(4);

  /**
   * float_epsilon
   * When absolute value of two real numbers is under float_epsilon, they are
   * considered equivalent.
   */
  const double float_epsilon(0.0001);

}  // namespace vita

#endif  // VITA_H
