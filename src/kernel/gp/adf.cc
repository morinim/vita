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

#include "kernel/gp/adf.h"
#include "kernel/gp/mep/interpreter.h"

namespace vita
{
///
/// \param[in] ind individual whose code is used as ADF/ADT
///
template<class T>
adf_core<T>::adf_core(const T &ind) : code_(ind), id_(adf_count())
{
}

///
/// \param[in] prefix a string identifying adf type
/// \return           an unique identifier (string) for this ADF/ADT symbol
///
template<class T>
std::string adf_core<T>::name(const std::string &prefix) const
{
  return prefix + std::to_string(id_);
}

///
/// \return the code (program) of the ADF/ADT
///
template<class T>
const T &adf_core<T>::code() const
{
  return code_;
}

///
/// \return `true` if the object passes the internal consistency check
///
template<class T>
bool adf_core<T>::is_valid() const
{
  return code_.active_symbols() >= 2;
}

///
/// \param[in] ind the code for the ADF
/// \param[in] sv  categories of the function arguments
///
adf::adf(const i_mep &ind, cvect sv)
  : function("ADF", ind.category(), std::move(sv)), core_(ind)
{
  Expects(ind.active_symbols() >= 2);

  Ensures(is_valid());
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
/// \note
/// ADF functions need input parameters from a context (contrary to adt::eval).
///
value_t adf::eval(core_interpreter *i) const
{
  Expects(typeid(*i) == typeid(interpreter<i_mep>));

  return interpreter<i_mep>(&code(),
                            static_cast<interpreter<i_mep> *>(i)).run();
}

///
/// \return the name (unique identifier) of the ADF
///
std::string adf::name() const
{
  return core_.name(symbol::name());
}

///
/// \return `true` if the object passes the internal consistency check
///
bool adf::is_valid() const
{
  // No recursive calls.
  if (std::any_of(code().begin(), code().end(),
                  [this](const gene &g) { return g.sym == this; }))
    return false;

  return core_.is_valid() && function::is_valid();
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
  Expects(ind.active_symbols() >= 2);

  Ensures(is_valid());
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
/// \note
/// ADT hasn't input parameters so the context is ignored (contrary to
/// adf::eval).
///
value_t adt::eval(core_interpreter *) const
{
  return interpreter<i_mep>(&code()).run();
}

///
/// \return the name (unique identifier) of the ADT
///
std::string adt::name() const
{
  return core_.name(symbol::name());
}

///
/// \return `true` if the object passes the internal consistency check
///
bool adt::is_valid() const
{
  // No recursive calls.
  if (std::any_of(code().begin(), code().end(),
                  [this](const gene &g) { return g.sym == this; }))
    return false;

  return core_.is_valid() && terminal::is_valid();
}

///
/// \return the code (program) of the ADT
///
const i_mep &adt::code() const
{
  return core_.code();
}

}  // namespace vita
