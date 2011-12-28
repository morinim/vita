/**
 *
 *  \file individual.h
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

  typedef std::array<unsigned, 2> locus;

  ///
  /// \param[in] l1 first locus.
  /// \param[in] l2 second locus.
  /// \return \c true if \a l1 precedes \a l2 in lexicographic order
  ///         (http://en.wikipedia.org/wiki/Lexicographical_order).
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

}  // namespace vita

#endif  // LOCUS_H
