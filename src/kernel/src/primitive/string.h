/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_STRING_PRIMITIVE_H)
#define      VITA_STRING_PRIMITIVE_H

#include <algorithm>
#include <cstdlib>
#include <string>

#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"

namespace vita::str
{

///
/// String comparison for equality.
///
class ife : public function
{
public:
  explicit ife(const cvect &c)
    : function("SIFE", c[1], {c[0], c[0], c[1], c[1]})
  { Expects(c.size() == 2); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case c_format:
    case cpp_format:
    case mql_format:     return "(%%1%% == %%2%% ? %%3%% : %%4%%)";
    case python_format:  return "(%%3%% if %%1%% == %%2%% else %%4%%)";
    default:             return function::display();
    }
  }

  std::any eval(core_interpreter *ci) const final
  {
    auto *i(static_cast<interpreter<i_mep> *>(ci));

    const std::any v0(i->fetch_arg(0));
    if (!v0.has_value())  return v0;

    const std::any v1(i->fetch_arg(1));
    if (!v1.has_value())  return v1;

    if (std::any_cast<std::string>(v0) == std::any_cast<std::string>(v1))
      return i->fetch_arg(2);
    else
      return i->fetch_arg(3);
  }
};

}  // namespace vita::str

#endif  // include guard
