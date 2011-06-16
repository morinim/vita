/**
 *
 *  \file symbol.cc
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
