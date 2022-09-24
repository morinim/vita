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

#if !defined(VITA_SYMBOL_H)
#define      VITA_SYMBOL_H

#include <limits>
#include <string>

#include "kernel/vitafwd.h"
#include "kernel/common.h"
#include "kernel/core_interpreter.h"

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
  /// Symbol rendering format.
  enum format {c_format, cpp_format, mql_format, python_format, sup_format};

  explicit symbol(const std::string &, category_t = 0);
  virtual ~symbol() = default;

  virtual unsigned arity() const = 0;
  virtual bool input() const;

  [[nodiscard]] category_t category() const;
  [[nodiscard]] opcode_t opcode() const;
  [[nodiscard]] bool terminal() const;

  void category(category_t);

  [[nodiscard]] std::string name() const;

  /// Calculates the value of / performs the action associated with the symbol
  /// (it's implementation specific).
  virtual value_t eval(symbol_params &) const = 0;

  [[nodiscard]] double penalty(core_interpreter *) const;

  virtual bool is_valid() const;

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
/// Changes the category of a symbol.
///
/// \param[in] c the new category
///
/// \remark Should be called only for symbols with undefined category.
///
inline void symbol::category(category_t c)
{
  Expects(category_ == undefined_category);
  Expects(c != category_);

  category_ = c;
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
/// \return `true` if this symbol is a `terminal`
///
inline bool symbol::terminal() const
{
  return arity() == 0;
}

}  // namespace vita

#endif  // include guard
