/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_BOOL_PRIMITIVE_H)
#define      VITA_BOOL_PRIMITIVE_H

#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>

#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"

namespace vita { namespace boolean
{
  class zero : public terminal
  {
  public:
    zero() : terminal("0", sym_bool, false, false, default_weight) {}

    virtual std::string display() const override { return "0"; }

    virtual any eval(interpreter<individual> *) const override { return false; }
  };

  class one : public terminal
  {
  public:
    one() : terminal("1", sym_bool, false, false, default_weight) {}

    virtual std::string display() const override { return "1"; }

    virtual any eval(interpreter<individual> *) const override { return true; }
  };

  class and : public function
  {
  public:
    and() : function("AND", sym_bool, 2, function::default_weight, true) {}

    virtual any eval(interpreter<individual> *i) const override
    {
      return any_cast<bool>(i->eval(0)) && any_cast<bool>(i->eval(1));
    }
  };

  class not : public function
  {
  public:
    not() : function("NOT", sym_bool, 1) {}

    virtual any eval(interpreter<individual> *i) const override
    { return !any_cast<bool>(i->eval(0)); }
  };

  class or : public function
  {
  public:
    or() : function("OR", sym_bool, 2, function::default_weight, true) {}

    virtual any eval(interpreter<individual> *i) const override
    {
      return any_cast<bool>(i->eval(0)) || any_cast<bool>(i->eval(1));
    }
  };
} }  // namespace vita::boolean

#endif  // Include guard
