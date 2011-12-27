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

#include <iostream>

#include "kernel/vita.h"

namespace vita
{

  ///
  /// These are the coordinates (locus) of a gene in the genome.
  ///
  struct loc_t
  {
    bool operator==(const loc_t &l) const
    { return index == l.index && category == l.category; }

    bool operator!=(const loc_t &l) const
    { return index != l.index || category != l.category; }

    bool operator<(const loc_t &l) const
    { return index < l.index || (index == l.index && category < l.category); }

    unsigned      index;
    category_t category;
  };

  ///
  /// \param[out] s output stream.
  /// \param[in] l locus to print.
  /// \return output stream including \a k.
  ///
  inline
  std::ostream &operator<<(std::ostream &s, const loc_t &l)
  {
    s << '(' << l.index << ',' << l.category << ')';

    return s;
  }
}  // namespace vita

#endif  // LOCUS_H
