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

#if !defined(VITA_SYMBOL_H)
#define      VITA_SYMBOL_H

#include <string>

#include "kernel/any.h"
#include "kernel/vitafwd.h"

namespace vita
{
///
/// GP assembles variable length program structures from basic units called
/// functions and terminals. Functions perform operations on their inputs,
/// which are either terminals or output from other functions.
/// Together functions and terminals are referred to as symbols.
///
class symbol
{
public:
  symbol(const std::string &, category_t);

  virtual unsigned arity() const = 0;
  bool associative() const;
  bool auto_defined() const;
  category_t category() const;
  bool input() const;
  opcode_t opcode() const;
  bool parametric() const;
  bool terminal() const;

  virtual std::string display() const;
  virtual std::string display(double) const;
  virtual double init() const;

  /// Calculates the value of / performs the action associated with the symbol
  /// (it's implementation specific).
  virtual any eval(core_interpreter *) const = 0;

  double penalty(core_interpreter *) const;

  virtual bool debug() const;

public:  // Public data members
  /// Weight is used by the symbol_set::roulette method to control the
  /// probability of extraction of the symbol.
  unsigned weight;

  /// This is the default weight. Weights are used by the
  /// symbol_set::roulette method to control the probability of extraction of
  /// the symbols.
  static constexpr decltype(weight) k_base_weight{100};

protected:  // Protected data members
  bool associative_;
  bool auto_defined_;
  bool input_;
  bool parametric_;

private:  // NVI template methods
  virtual double penalty_nvi(core_interpreter *) const;

private:  // Private data members
  static opcode_t opc_count_;

  opcode_t opcode_;

  category_t category_;

  std::string name_;
};

///
/// \param[in] ci interpreter used for symbol's constraints evaluation.
/// \return 0
///
/// Return a penalty based on symbol-specific broken constraints:
/// - 0 states that no constraint penalty is applied;
/// - larger values specify larger penalties.
///
inline double symbol::penalty(core_interpreter *ci) const
{
  return penalty_nvi(ci);
}

///
/// \return `0.0`.
///
/// This function is used to initialize the symbol's internal parameter.
/// Derived classes should redefine the init member function in a
/// meaningful way.
///
inline double symbol::init() const
{
  return 0.0;
}

///
/// \return true if the function is associative.
///
/// The associative law of arithmetic: if OP is associative then
/// > a OP (b OP c) = (a OP b) OP c = a OP b OP c
///
/// This information can be used for optimization and visualization.
///
/// \note
/// Terminals haven't arguments and cannot be associative.
///
inline bool symbol::associative() const
{
  return associative_;
}

///
/// \return `true` if the symbol has been automatically defined (e.g.
///         ADF / ADT), `false` otherwise (this is the default value).
///
inline bool symbol::auto_defined() const
{
  return auto_defined_;
}

///
/// \return the category of the symbol.
///
/// In strongly typed GP, every terminal has a type (i.e. category) and every
/// function has types for each of its arguments and a type for its return
/// value.
///
inline category_t symbol::category() const
{
  return category_;
}

///
/// \return `true` if the symbol is an input variable.
///
/// An input variable is a feature from the learning domain. Only terminal
/// can be input variable.
///
inline bool symbol::input() const
{
  return input_;
}

///
/// \return the opcode of the symbol.
///
/// The opcode is a fast way to uniquely identify a symbol and is primarily
/// used for hashing.
/// The other way to identify a symbol is by its name (std::string). The name
/// is often a better way since the opcode of a symbol can vary between
/// executions.
///
inline opcode_t symbol::opcode() const
{
  return opcode_;
}

///
/// \return true for parametric symbols.
///
/// A parametric symbol needs an additional argument to be evaluated.
///
/// A value for this argument is stored in every gene where the parametric
/// symbol is used and it's fetched at run-time.
///
/// \note
/// Functions are never parametric, terminals can be.
///
inline bool symbol::parametric() const
{
  return parametric_;
}

///
/// \return `true` if this symbol is a `terminal`.
///
inline bool symbol::terminal() const
{
  return arity() == 0;
}

}  // namespace vita

#endif  // Include guard
