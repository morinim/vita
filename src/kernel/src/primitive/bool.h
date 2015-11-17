/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_BOOL_PRIMITIVE_H)
#define      VITA_BOOL_PRIMITIVE_H

#include <cstdlib>
#include <string>

#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/terminal.h"

namespace vita
{
namespace boolean
{
class zero : public terminal
{
public:
  explicit zero(const cvect &c) : terminal("0", c[0])
  { assert(c.size() == 1); }

  virtual std::string display() const override { return "0"; }

  virtual any eval(core_interpreter *) const override { return any(false); }
};

class one : public terminal
{
public:
  explicit one(const cvect &c) : terminal("1", c[0])
  { assert(c.size() == 1); }

  virtual std::string display() const override { return "1"; }

  virtual any eval(core_interpreter *) const override { return any(true); }
};

class l_and : public function
{
public:
  explicit l_and(const cvect &c) : function("AND", c[0], {c[0], c[0]})
  { assert(c.size() == 1); }

  virtual bool associative() const override { return true; }

  virtual any eval(core_interpreter *ci) const override
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    return any(any_cast<bool>(i->fetch_arg(0)) &&
               any_cast<bool>(i->fetch_arg(1)));
  }
};

class l_not : public function
{
public:
  explicit l_not(const cvect &c) : function("NOT", c[0], {c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    return any(!any_cast<bool>(i->fetch_arg(0)));
  }
};

class l_or : public function
{
public:
  explicit l_or(const cvect &c) : function("OR", c[0], {c[0], c[0]})
  { assert(c.size() == 1); }

  virtual bool associative() const override { return true; }

  virtual any eval(core_interpreter *ci) const override
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    return any(any_cast<bool>(i->fetch_arg(0)) ||
               any_cast<bool>(i->fetch_arg(1)));
  }
};

}  // namespace boolean
}  // namespace vita

#endif  // Include guard
