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
  /// \param[in] in input stream.
  /// \return \c true if gene was loaded correctly.
  ///
  bool gene::load(std::istream &in, const symbol_set &ss)
  {
    /*
    opcode_t opcode;
    in >> opcode;

    const symbol_ptr s(ss.decode(opcode));

    if (!in.good() || !s)
      return false;
    */
  }

  ///
  /// \param[out] out output stream.
  /// \return \c true if gene was saved correctly.
  ///
  bool gene::save(std::ostream &out) const
  {
/*
    out << sym->opcode();

    if (sym->parametric())
      out << ' ' << par;

    for (size_t i(0); i < sym->arity(); ++i)
      out << ' ' << args;

    out << std::endl;
*/
    return out.good();
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

    const size_t n(sym->arity());
    for (size_t i(0); i < n; ++i)
      if (args[i] != g.args[i])
        return false;

    return true;
  }
}  // namespace vita
