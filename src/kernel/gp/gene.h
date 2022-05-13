/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GENE_H)
#define      VITA_GENE_H

#include "kernel/random.h"
#include "kernel/gp/function.h"
#include "kernel/gp/locus.h"
#include "kernel/gp/terminal.h"
#include "utility/small_vector.h"
#include "utility/utility.h"

namespace vita
{
///
/// A gene is a unit of heredity in a living organism.
///
/// \tparam K the maximum `function`'s number of arguments
///
/// The class `gene` is the building block of a `i_mep` individual.
///
template<unsigned K>
class basic_gene
{
public:
  basic_gene() {}
  explicit basic_gene(const terminal &);
  basic_gene(const std::pair<symbol *, std::vector<index_t>> &);
  basic_gene(const symbol &, index_t, index_t);

  locus locus_of_argument(std::size_t i) const;
  small_vector<locus, K> arguments() const;

  // Types and constants.
  using packed_index_t = std::uint16_t;
  using arg_pack = small_vector<packed_index_t, K>;

  enum : decltype(K) {k_args = K};

  // Public data members.
  const symbol    *sym;
  terminal_param_t par;
  arg_pack        args;

private:
  void init_if_parametric();
};

template<unsigned K>
std::ostream &operator<<(std::ostream &, const basic_gene<K> &);

template<unsigned K>
bool operator==(const basic_gene<K> &, const basic_gene<K> &);
template<unsigned K>
bool operator!=(const basic_gene<K> &, const basic_gene<K> &);

///
/// A basic_gene with the standard size.
///
/// A gene supports functions with more than 4 arguments (with a performance
/// penality).
///
using gene = basic_gene<4>;

#include "kernel/gp/gene.tcc"
}  // namespace vita

#endif  // include guard
