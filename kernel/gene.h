/**
 *
 *  \file gene.h
 *
 *  \author Manlio Morini
 *  \date 2011/05/11
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(GENE_H)
#define      GENE_H

#include "symbol_set.h"

namespace vita
{

  ///
  /// Maximum number of arguments for a function.
  ///
  const unsigned gene_args(4);

  ///
  /// A gene is a unit of heredity in a living organism. The \c struct \a gene
  /// is the building block for an \a individual.
  ///
  struct gene
  {
    gene() {};
    gene(const symbol_set &);
    gene(const symbol_set &, unsigned, unsigned);
    gene(const symbol_set &, unsigned);

    bool operator==(const gene &) const;
    bool operator!=(const gene &g) const { return !(*this == g); };

    const symbol         *sym;
    union
    {
      int                 par;
      locus_t args[gene_args];
    };
  };

}  // namespace vita

#endif  // GENE_H
