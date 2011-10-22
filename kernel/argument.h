/**
 *
 *  \file argument.h
 *
 *  Copyright 2011 EOS di Manlio Morini.
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

#if !defined(ARGUMENT_H)
#define      ARGUMENT_H

#include <string>

#include "kernel/vita.h"
#include "kernel/terminal.h"

namespace vita
{
  class interpreter;

  ///
  /// \a argument is a special \a terminal used by \a adf functions for
  /// input parameters passing.
  ///
  class argument : public terminal
  {
  public:
    explicit argument(unsigned);

    std::string display() const;

    unsigned index() const;

    boost::any eval(interpreter *) const;

    bool check() const;

  private:
    const unsigned index_;
  };
}  // namespace vita

#endif  // ARGUMENT_H
