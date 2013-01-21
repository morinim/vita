/**
 *
 *  \file gene.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "gene.h"
#include "function.h"
#include "random.h"

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

  gene::gene(const symbol_ptr &s, index_t from, index_t sup) : sym(s)
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
