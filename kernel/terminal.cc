/**
 *
 *  \file terminal.cc
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

#include "kernel/terminal.h"

namespace vita
{
  /// This is the default value for the terminal constructor. Weights are
  /// used by the symbol_set::roulette method to control the probability of
  /// extraction of the symbols.
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
    : symbol(dis, t, w), parametric_(par), input_(in)
  {
    assert(check());
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool terminal::check() const
  {
    return symbol::check();
  }
}  // Namespace vita
