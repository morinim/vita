/**
 *
 *  \file individual.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(LOCUS_H)
#define      LOCUS_H

#include <array>
#include <iostream>

#include "boost/multi_array.hpp"

#include "kernel/gene.h"

namespace vita
{

  static_assert(sizeof(index_t) <= sizeof(unsigned),
                "index_t sizes expected to be <= unsigned");
  static_assert(sizeof(category_t) <= sizeof(unsigned),
                "category_t sizes expected to be <= unsigned");

  enum {locus_index = 0, locus_category = 1};

  /// locus[index in the genome][type of the symbol].
  typedef std::array<unsigned, 2> locus;

  ///
  /// \param[in] l1 first locus.
  /// \param[in] l2 second locus.
  /// \return \c true if \a l1 precedes \a l2 in lexicographic order
  ///         (http://en.wikipedia.org/wiki/Lexicographical_order).
  ///
  /// This operator is required by the STL std::map container.
  ///
  inline
  bool operator<(const locus &l1, const locus &l2)
  { return l1[0] < l2[0] || (l1[0] == l2[0] && l1[1] < l2[1]); }

  ///
  /// \param[out] s output stream.
  /// \param[in] l locus to print.
  /// \return output stream including \a l.
  ///
  inline
  std::ostream &operator<<(std::ostream &s, const locus &l)
  {
    return s << '(' << l[0] << ',' << l[1] << ')';
  }

  ///
  /// \param[in] l starting locus.
  /// \param[in] i displacement.
  /// \return a new locus obtained from \a l incrementing index component by
  ///         \a i (and not changing category component).
  ///
  inline
  locus operator+(const locus &l, unsigned i)
  {
    return locus{{l[0] + i, l[1]}};
  }

}  // namespace vita

#endif  // LOCUS_H
