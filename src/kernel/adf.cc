/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/adf.h"
#include "kernel/interpreter.h"

namespace vita
{
///
/// \param[in] ind the code for the ADF
/// \param[in] sv  categories of the function arguments
///
adf::adf(const i_mep &ind, cvect sv)
  : function("ADF", ind.category(), std::move(sv)), core_(ind)
{
  Expects(ind.debug());
  Expects(ind.active_symbols() >= 2);

  Ensures(debug());
}

///
/// \return `true`
///
bool adf::auto_defined() const
{
  return true;
}

///
/// \param[in] i the context in which this ADF is evaluated
/// \return      the output of the ADF
///
/// Adf functions need input parameters from the a context (contrary to
/// adt::eval).
///
any adf::eval(core_interpreter *i) const
{
  assert(typeid(*i) == typeid(interpreter<i_mep>));

  return interpreter<i_mep>(&code(),
                            static_cast<interpreter<i_mep> *>(i)).run();
}

///
/// \return the name of the ADF
///
std::string adf::display() const
{
  return core_.display(symbol::display());
}

///
/// \return `true` if the object passes the internal consistency check
///
bool adf::debug() const
{
  const auto cod(code());
  // No recursive calls.
  for (const auto &g : cod)
    if (g.sym == this)
      return false;

  if (!core_.debug())
    return false;

  return function::debug();
}

///
/// \return the code (program) of the ADF
///
const i_mep &adf::code() const
{
  return core_.code();
}

///
/// \param[in] ind the code for the ADT
///
adt::adt(const i_mep &ind) : terminal("ADT", ind.category()), core_(ind)
{
  Expects(ind.debug());
  Expects(ind.active_symbols() >= 2);

  Ensures(debug());
}

///
/// \return `true`
///
bool adt::auto_defined() const
{
  return true;
}

///
/// \return the output of the ADT
///
/// Adt hasn't input parameters so the context is ignored (contrary to
/// adf::eval).
///
any adt::eval(core_interpreter *) const
{
  return interpreter<i_mep>(&code()).run();
}

///
/// \return the name of the ADT
///
std::string adt::display() const
{
  return core_.display(symbol::display());
}

///
/// \return `true` if the object passes the internal consistency check
///
bool adt::debug() const
{
  const auto cod(code());
  // No recursive calls.
  for (const auto &g : cod)
    if (g.sym == this)
      return false;

  if (!core_.debug())
    return false;

  return terminal::debug();
}

///
/// \return the code (program) of the ADT
///
const i_mep &adt::code() const
{
  return core_.code();
}

}  // namespace vita
