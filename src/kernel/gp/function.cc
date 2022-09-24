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

#include "kernel/gp/function.h"

namespace vita
{
///
/// \param[in] dis  string representation of the function (e.g. for the plus
///                 function it could by "ADD" or "+")
/// \param[in] c    category of the function (i.e. the category of the output
///                 value)
/// \param[in] args input parameters (type and number) of the function (in
///                 C++ they are called the "function signature")
///
function::function(const std::string &dis, category_t c, cvect args)
  : symbol(dis, c), argt_(std::move(args))
{
  Ensures(is_valid());
}

///
/// \return the name of the function
///
/// \warning
/// Specific functions have to specialize this method to support different
/// output formats.
///
std::string function::display(format) const
{
  std::string args("%%1%%");
  for (unsigned i(1); i < arity(); ++i)
    args += ",%%" + std::to_string(i + 1) + "%%";

  return name() + "(" + args + ")";
}

///
/// \return `true` if the object passes the internal consistency check
///
bool function::is_valid() const
{
  if (!arity())  // this is a function, we want some argument...
    return false;

  return symbol::is_valid();
}

}  // namespace vita
