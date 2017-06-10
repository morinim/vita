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

#include "kernel/symbol.h"
#include "kernel/gene.h"

namespace vita
{
opcode_t symbol::opc_count_(0);

///
/// \param[in] name name of the symbol
/// \param[in] c    category of the symbol
///
/// \note
/// By default a symbol:
/// - is NOT associative;
/// - is NOT auto defined;
/// - is NOT an input;
/// - is NOT parametric.
/// These are safe values for derived classes.
///
/// \warning
/// Since the name of the symbol is used for serialization, it must be
/// unique. Even the opcode is unique, but it can change between executions.
///
symbol::symbol(const std::string &name, category_t c)
  : opcode_(opc_count_++), category_(c), name_(name)
{
  Ensures(debug());
}

///
/// \return the name of the symbol
///
std::string symbol::display() const
{
  return name_;
}

///
/// \param[in] v additional informations regarding parametric symbols
/// \return      a string representing the symbol
///
/// This function is called for parametric symbols only. The `v` argument is
/// used to build a more meaningful name for the symbol (i.e. for a numeric
/// terminal it's better to print 123 than "NUMBER").
///
std::string symbol::display(double v) const
{
  Expects(parametric());

  return display() + "_" + std::to_string(v);
}

///
/// \return `0.0`
///
double symbol::penalty_nvi(core_interpreter *) const
{
  return 0.0;
}

///
/// \return `true` if the object passes the internal consistency check
///
bool symbol::debug() const
{
  return display().size() > 0;
}

}  // namespace vita
