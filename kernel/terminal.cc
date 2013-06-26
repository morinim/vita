/**
 *
 *  \file terminal.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "kernel/terminal.h"

namespace vita
{
  ///
  /// \param[in] dis string printed to identify the terminal.
  /// \param[in] c category of the terminal.
  /// \param[in] par true if the terminal is parametric.
  /// \param[in] in true if the terminal is an input value.
  /// \param[in] w weight used for symbol frequency control.
  ///
  terminal::terminal(const std::string &dis, category_t c, bool in, bool par,
                     unsigned w)
    : symbol(dis, c, w), parametric_(par), input_(in)
  {
    assert(debug());
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool terminal::debug() const
  {
    return symbol::debug();
  }
}  // Namespace vita
