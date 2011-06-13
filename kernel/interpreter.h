/**
 *
 *  \file interpreter.h
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

#if !defined(INTERPRETER_H)
#define      INTERPRETER_H

#include <boost/any.hpp>

#include <vector>

#include "kernel/vita.h"

namespace vita
{
  class individual;

  class interpreter
  {
  public:
    explicit interpreter(const individual &, interpreter *const = 0);

    boost::any run();

    boost::any eval();
    boost::any eval(unsigned);
    boost::any eval_adf_arg(unsigned);

    bool check() const;

  private:
    // Instruction pointer.
    unsigned ip_;

    interpreter *const context_;

    const individual &ind_;

    struct cache_elem
    {
      bool       empty;
      boost::any value;
    };

    mutable std::vector<cache_elem> cache_;
    mutable std::vector<boost::any> context_cache_;
  };

  ///
  /// \example example5.cc
  /// Output value calculation for an individual.
  ///
}  // namespace vita

#endif  // INTERPRETER_H
