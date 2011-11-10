/**
 *
 *  \file bool.h
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

#if !defined(BOOL_PRIMITIVE_H)
#define      BOOL_PRIMITIVE_H

#include <boost/any.hpp>

#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>

#include "kernel/vita.h"
#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"

namespace vita
{
  namespace boolean
  {
    class variable : public terminal
    {
    public:
      explicit variable(const std::string &name)
        : terminal(name, sym_bool, true) {}

      boost::any eval(vita::interpreter *) const { return val; }

      bool val;
    };

    class zero : public terminal
    {
    public:
      zero() : terminal("0", sym_bool, false, false, default_weight*3) {}

      std::string display() const { return "0"; }

      boost::any eval(vita::interpreter *) const { return false; }
    };

    class one : public terminal
    {
    public:
      one() : terminal("1", sym_bool, false, false, default_weight*3) {}

      std::string display() const { return "1"; }

      boost::any eval(vita::interpreter *) const { return true; }
    };

    class and : public function
    {
    public:
      and() : function("AND", sym_bool, 2, function::default_weight, true) {}

      boost::any eval(vita::interpreter *i) const
      {
        return boost::any_cast<bool>(i->eval(0)) &&
          boost::any_cast<bool>(i->eval(1));
      }
    };

    class not : public function
    {
    public:
      not() : function("NOT", sym_bool, 1) {}

      boost::any eval(vita::interpreter *i) const
      { return !boost::any_cast<bool>(i->eval(0)); }
    };

    class or : public function
    {
    public:
      or() : function("OR", sym_bool, 2, function::default_weight, true) {}

      boost::any eval(vita::interpreter *i) const
      {
        return boost::any_cast<bool>(i->eval(0)) ||
          boost::any_cast<bool>(i->eval(1));
      }
    };
  }  // namespace boolean
}  // namespace vita

#endif  // BOOL_PRIMITIVE_H
