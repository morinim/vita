/**
 *
 *  \file argument.cc
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

#include "kernel/argument.h"
#include "kernel/gene.h"
#include "kernel/interpreter.h"

namespace vita
{
  ///
  /// \param[in] n argument index.
  ///
  /// An \a adf_n function may have up to \a k_args arguments.
  ///
  argument::argument(unsigned n)
    : terminal("ARG", sym_void, false, false, 0), index_(n)
  {
    assert(check());
  }

  ///
  /// \return the index of the argument.
  ///
  unsigned argument::index() const
  {
    return index_;
  }

  ///
  /// \return the string representiation of the argument.
  ///
  std::string argument::display() const
  {
    std::ostringstream s;
    s << "ARG" << '_' << index_;
    return s.str();
  }

  ///
  /// \param[in] agent current interpreter
  /// \return the value of the argument.
  ///
  boost::any argument::eval(interpreter *agent) const
  {
    return agent->eval_adf_arg(index_);
  }

  ///
  /// \return \a true if the object passes the internal consistency check.
  ///
  bool argument::check() const
  {
    return index_ < gene::k_args && terminal::check();
  }
}  // Namespace vita
