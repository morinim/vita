/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
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

#include "kernel/gp/function.h"
#include "kernel/gp/mep/interpreter.h"
#include "kernel/gp/terminal.h"
#include "kernel/random.h"

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

  value_t eval(symbol_params &args) const final
  {
    const auto v0(args[0]);
    if (!has_value(v0))  return v0;

    const auto v1(args[1]);
    if (!has_value(v1))  return v1;

    if (v0 == v1)
      return args[2];

    return args[3];
  }
};

}  // namespace vita::str

#endif  // include guard
