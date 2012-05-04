/**
 *
 *  \file bool.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
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
