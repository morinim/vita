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

#include "kernel/gp/argument.h"
#include "kernel/gp/gene.h"
#include "kernel/gp/interpreter.h"
#include "kernel/gp/mep/i_mep.h"

namespace vita
{
///
/// `n`-th argument constructor.
///
/// \param[in] n argument index
///
/// An adf function may have up to `k_args` arguments. Arguments' category is
/// special: they haven't a type because arguments are communication channels
/// among ADF functions and their calling environments.
/// So the type that is travelling on channel `i` (`argument(i)`) varies
/// depending on the function being evaluated (instead, adf functions have a
/// precise, fixed signature).
///
argument::argument(unsigned n) : terminal("ARG", category_t()), index_(n)
{
  Ensures(is_valid());
}

///
/// \return the index of the argument
///
unsigned argument::index() const
{
  return index_;
}

///
/// \return the name of the argument
///
std::string argument::name() const
{
  return "ARG_" + std::to_string(index_);
}

///
/// \param[in] agent current interpreter
/// \return          the value of the argument
///
value_t argument::eval(core_interpreter *agent) const
{
  Expects(typeid(*agent) == typeid(interpreter<i_mep>));

  return static_cast<interpreter<i_mep> *>(agent)->fetch_adf_arg(index_);
}

///
/// \return `true` if the object passes the internal consistency check
///
bool argument::is_valid() const
{
  return index_ < gene::k_args && terminal::is_valid();
}

}  // namespace vita
