/**
 *
 *  \file gene.h
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

#if !defined(GENE_H)
#define      GENE_H

#include "kernel/symbol_set.h"

namespace vita
{
  ///
  /// A gene is a unit of heredity in a living organism. The \c struct \a gene
  /// is the building block for an \a individual.
  ///
  struct gene
  {
    /// Maximum number of arguments for a function.
    enum {k_args = 4};

    gene() {}
    gene(const symbol_set &);
    gene(const symbol_set &, unsigned, unsigned);
    gene(const symbol_set &, unsigned);

    bool operator==(const gene &) const;
    bool operator!=(const gene &g) const { return !(*this == g); }

    symbol_ptr         sym;
    union
    {
      int              par;
      locus_t args[k_args];
    };
  };
}  // namespace vita

#endif  // GENE_H
