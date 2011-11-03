/**
 *
 *  \file gene.cc
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

#include "kernel/gene.h"
#include "kernel/random.h"
#include "kernel/symbol.h"

namespace vita
{
  ///
  /// \param[in] sset the set of symbols used to build individuals.
  ///
  gene::gene(const symbol_set &sset)
  {
    sym = sset.roulette(true);

    if (sym->parametric())
      par = sym->init();
  }

  ///
  /// \param[in] sset the set of symbols used to build individuals.
  /// \param[in] from minimum index for function arguments.
  /// \param[in] sup upper bound for the indexes of function arguments.
  ///
  gene::gene(const symbol_set &sset, unsigned from, unsigned sup)
  {
    assert(from <= sup);
    sym = sset.roulette(from == sup);

    if (sym->parametric())
      par = sym->init();
    else
    {
      const unsigned arity(sym->arity());
      for (unsigned j(0); j < arity; ++j)
        args[j] = random::between<unsigned>(from, sup);
    }
  }

  ///
  /// \param[in] sset the set of symbols used to build individuals.
  /// \param[in] i index of a sticky symbol in the symbol_set.
  ///
  gene::gene(const symbol_set &sset, unsigned i)
  {
    sym = sset.get_sticky(i);
    if (sym->parametric())
      par = sym->init();
  }

  ///
  /// \param[in] g second term of comparison.
  /// \return \c true if \c this == \a g
  ///
  bool gene::operator==(const gene &g) const
  {
    if (sym != g.sym)
      return false;

    if (sym->parametric())
      return par == g.par;

    const unsigned n(sym->arity());
    for (unsigned i(0); i < n; ++i)
      if (args[i] != g.args[i])
        return false;

    return true;
  }
}  // namespace vita
