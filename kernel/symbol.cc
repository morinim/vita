/**
 *
 *  \file symbol.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "kernel/symbol.h"

namespace vita
{
  unsigned symbol::opc_count_(0);

  ///
  /// \return the name of the symbol.
  ///
  std::string symbol::display() const
  {
    return display_;
  }

  ///
  /// \param[in] v
  /// \return a string representing the symbol.
  ///
  /// This function is called by parametric symbols only. The \a v argument is
  /// used to build a more meaningful name for the symbol.
  ///
  std::string symbol::display(int v) const
  {
    assert(parametric());

    std::ostringstream s;
    s << display_ << '_' << v;
    return s.str();
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool symbol::check() const
  {
    return display_.size() > 0;
  }
}  // Namespace vita
