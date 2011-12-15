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
#include "kernel/function.h"
#include "kernel/random.h"

namespace vita
{
  ///
  /// \param[in] t a terminal.
  ///
  /// A new gene built from terminal \a t.
  ///
  gene::gene(const symbol_ptr &t) : sym(t)
  {
    assert(sym->terminal());

    if (sym->parametric())
      par = sym->init();
  }

  gene::gene(const symbol_ptr &s, unsigned from, unsigned sup) : sym(s)
  {
    assert(from < sup);

    if (sym->parametric())
      par = sym->init();
    else
    {
      const unsigned arity(sym->arity());
      for (unsigned i(0); i < arity; ++i)
        args[i] = random::between(from, sup);
    }
  }

  ///
  /// \param[in] sset the set of symbols used to build individuals.
  /// \param[in] where an array used for category -> locus tracking.
  ///
  gene::gene(const symbol_ptr &s,
             const std::vector<std::vector<unsigned>> &where)
    : sym(s)
  {
    if (sym->parametric())
      par = sym->init();
    else
    {
      const unsigned arity(sym->arity());
      for (unsigned i(0); i < arity; ++i)
      {
        const category_t c(std::static_pointer_cast<function>(sym)
                           ->arg_category(i));
        args[i] = random::element(where[c]);
      }
    }
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
