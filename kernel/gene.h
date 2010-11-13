/**
 *
 *  \file gene.h
 *
 *  \author Manlio Morini
 *  \date 2010/11/13
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(GENE_H)
#define      GENE_H

#include "symbol_set.h"

namespace vita
{

  /**
   * gene_args
   * Maximum number of arguments for a vita function.
   */
  const unsigned gene_args(4);

  struct gene
  {
    gene() {};
    gene(const symbol_set &);
    gene(const symbol_set &, unsigned, unsigned);

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
