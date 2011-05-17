/**
 *
 *  \file terminal.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */

#include "terminal.h"

namespace vita
{

  unsigned terminal::default_weight(100);

  ///
  /// \param[in] dis string printed to identify the terminal.
  /// \param[in] t type of the terminal.
  /// \param[in] par true if the terminal is parametric.
  /// \param[in] in true if the terminal is an input value.
  /// \param[in] w weight used for symbol frequency control.
  ///
  terminal::terminal(const std::string &dis, symbol_t t, bool in, bool par,
                     unsigned w)
    : symbol(dis,t,w), _parametric(par), _input(in)
  {
    assert(check());
  }

  ///
  /// \return true if the object passes the internal consistency check.
  ///
  bool
  terminal::check() const
  {
    return symbol::check();
  }

}  // Namespace vita
