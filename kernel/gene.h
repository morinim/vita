/**
 *
 *  \file gene.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(GENE_H)
#define      GENE_H

#include <vector>

#include "vita.h"

namespace vita
{
  class symbol_set;

  ///
  /// A gene is a unit of heredity in a living organism. The \c struct \a gene
  /// is the building block for an \a individual.
  ///
  struct gene
  {
    /// Maximum number of arguments for a function.
    enum {k_args = 4};

    gene() {}
    explicit gene(const symbol_ptr &);
    gene(const symbol_ptr &, unsigned, unsigned);

    bool operator==(const gene &) const;
    bool operator!=(const gene &g) const { return !(*this == g); }

    // Data members.
    symbol_ptr         sym;
    union
    {
      int              par;
      index_t args[k_args];
    };

  private:  // Serialization.
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &, unsigned);
  };

  ///
  /// \see \c boost::serialization
  ///
  template<class Archive>
  void gene::serialize(Archive &ar, unsigned)
  {
    static_assert(
      sizeof(par) <= sizeof(args),
      "sizeof(int) size expected to be <= sizeof(index_t[k_args] for union serialization)");

    ar & sym;
    ar & args;
  }
}  // namespace vita

#endif  // GENE_H
