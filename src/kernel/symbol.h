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

#if !defined(VITA_SYMBOL_H)
#define      VITA_SYMBOL_H

#include <string>

#include "kernel/vitafwd.h"
#include "utility/any.h"

namespace vita
{
/// This is the type used as key for symbol identification.
using opcode_t = unsigned;

///
/// Together functions and terminals are referred to as symbols.
///
/// GP assembles variable length program structures from basic units called
/// functions and terminals. Functions perform operations on their inputs,
/// which are either terminals or output from other functions.
///
class symbol
{
public:
  symbol(const std::string &, category_t);

  virtual unsigned arity() const = 0;
  virtual bool auto_defined() const;
  virtual bool input() const;
  virtual bool parametric() const;

  category_t category() const;
  opcode_t opcode() const;
  bool terminal() const;

  virtual std::string display() const;
  virtual std::string display(double) const;
  virtual double init() const;

  /// Calculates the value of / performs the action associated with the symbol
  /// (it's implementation specific).
  virtual any eval(core_interpreter *) const = 0;

  double penalty(core_interpreter *) const;

  virtual bool debug() const;

private:
  // NVI template methods
  virtual double penalty_nvi(core_interpreter *) const;

  // Private data members
  static opcode_t opc_count_;

  opcode_t opcode_;

  category_t category_;

  std::string name_;
};

///
/// Used for automatic calculation of penalities due to broken constraints.
///
/// \param[in] ci interpreter used for symbol's constraints evaluation
/// \return       a penalty based on symbol specific broken constraints
///
/// - `0.0` states that no constraint penalty is applied;
/// - larger values specify larger penalties.
///
inline double symbol::penalty(core_interpreter *ci) const
{
  return penalty_nvi(ci);
}

///
/// Used to initialize the symbol's internal parameter.
///
/// \return `0.0`
///
/// \remark
/// Derived classes should redefine the init member function in a meaningful
/// way.
///
inline double symbol::init() const
{
  return 0.0;
}

///
/// \return `true` if the symbol has been automatically defined (e.g.
///         ADF / ADT), `false` otherwise (this is the default value)
///
inline bool symbol::auto_defined() const
{
  return false;
}

///
/// The type (a.k.a. category) of the symbol
///
/// \return the category
///
/// In strongly typed GP every terminal and every function argument / return
/// value has a type (a.k.a. category).
///
inline category_t symbol::category() const
{
  return category_;
}

///
/// An input variable is a feature from the learning domain.
///
/// \return `true` if the symbol is an input variable
///
/// Only a terminal can be an input variable.
///
/// Default (safe) value is `false`.
///
inline bool symbol::input() const
{
  return false;
}

///
/// An opcode is a unique, numerical session ID for a symbol.
///
/// \return the opcode
///
/// The opcode is a fast way to uniquely identify a symbol and is primarily
/// used for hashing.
///
/// \remark
/// A symbol can be identified also by its name (a `std::string`). The name
/// is often a better option since it doesn't change among executions.
///
inline opcode_t symbol::opcode() const
{
  return opcode_;
}

///
/// A parametric symbol needs an additional parameter to be evaluated.
///
/// \return `true` for parametric symbols
///
/// Genes associated with parametric symbols store an additional parameter
/// fetched at run-time and used for symbol evaluation.
///
/// \note Functions cannot be parametric.
///
inline bool symbol::parametric() const
{
  return false;
}

///
/// \return `true` if this symbol is a `terminal`
///
inline bool symbol::terminal() const
{
  return arity() == 0;
}

}  // namespace vita

#endif  // include guard
