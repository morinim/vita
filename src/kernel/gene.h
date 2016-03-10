/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GENE_H)
#define      VITA_GENE_H

#include "kernel/locus.h"
#include "kernel/function.h"
#include "kernel/random.h"
#include "kernel/terminal.h"
#include "utility/small_vector.h"
#include "utility/utility.h"

namespace vita
{
///
/// \brief A gene is a unit of heredity in a living organism.
///
/// \tparam K the maximum `function`'s number of arguments.
///
/// The class `gene` is the building block of a `i_mep` individual.
///
template<unsigned K>
class basic_gene
{
public:
  basic_gene() {}
  explicit basic_gene(terminal *);
  basic_gene(const std::pair<symbol *, std::vector<index_t>> &);
  basic_gene(symbol *, index_t, index_t);

  locus arg_locus(unsigned) const;

  // Types and constants
  using param_type = double;
  using arg_pack = small_vector<std::uint16_t, K>;

  enum : decltype(K) {k_args = K};

  // Public data members
  const symbol *sym;
  param_type  par;
  arg_pack   args;
};

template<unsigned K>
std::ostream &operator<<(std::ostream &, const basic_gene<K> &);

template<unsigned K>
bool operator==(const basic_gene<K> &, const basic_gene<K> &);
template<unsigned K>
bool operator!=(const basic_gene<K> &, const basic_gene<K> &);

using gene = basic_gene<4>;

#include "kernel/gene.tcc"
}  // namespace vita

#endif  // Include guard
