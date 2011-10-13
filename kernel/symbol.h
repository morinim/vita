/**
 *
 *  \file symbol.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if !defined(SYMBOL_H)
#define      SYMBOL_H

#include <boost/any.hpp>

#include <sstream>
#include <string>

#include "kernel/vita.h"

namespace vita
{
  const symbol_t sym_void(0);
  const symbol_t sym_bool(1);
  const symbol_t sym_real(2);
  const symbol_t free_symbol(3);

  class interpreter;

  ///
  /// GP assembles variable length program structures from basic units called
  /// functions and terminals. Functions perform operations on their inputs,
  /// which are either terminals or output from other functions.
  /// Together functions and terminals are referred to as symbols (or nodes).
  ///
  class symbol
  {
  public:
    symbol(const std::string &, symbol_t, unsigned);

    opcode_t opcode() const;
    bool terminal() const;

    symbol_t type() const;

    virtual bool auto_defined() const;

    virtual std::string display() const;
    virtual std::string display(int) const;
    virtual int init() const;

    /// The associative law of arithmetic: if OP is associative then
    /// a OP (b OP c) = (a OP b) OP c = a OP b OP c
    /// This information can be used for optimization and visualization.
    virtual bool associative() const = 0;

    /// A parametric symbol needs an additional argument to be evaluated.
    /// A value for this argument is stored in every gene where the parametric
    /// symbol is used and it's fetched at run-time.
    /// Functions are never parametric; terminals can be parametric.
    virtual bool parametric() const = 0;

    /// The arity of a function is the number of inputs to or arguments of that
    /// funtion.
    virtual unsigned arity() const = 0;

    /// Calculates the value of / performs the action associated to the symbol
    /// (it is implementation specific).
    virtual boost::any eval(interpreter *) const = 0;

    bool check() const;

    /// Weights is used by the symbol_set::roulette method to control the
    /// probability of extraction of the symbol.
    unsigned weight;

  private:
    static unsigned opc_count_;

    const opcode_t     opcode_;

    const symbol_t       type_;

    const std::string display_;
  };

  ///
  /// \param[in] dis string used for printing.
  /// \param[in] t type of the symbol.
  /// \param[in] w weight (used for random selection).
  ///
  inline
  symbol::symbol(const std::string &dis, symbol_t t, unsigned w)
    : weight(w), opcode_(++opc_count_), type_(t), display_(dis)
  {
    assert(check());
  }

  ///
  /// \return 0.
  ///
  /// This function is used to initialize the symbol's internal parameter.
  /// Derived classes should redefine the init member function in a
  /// meaningful way.
  ///
  inline
  int symbol::init() const
  {
    return 0;
  }

  ///
  /// \return \c true if this symbol is a \c terminal.
  ///
  inline
  bool symbol::terminal() const
  {
    return arity() == 0;
  }

  ///
  /// \return the type of the \a symbol.
  ///
  inline
  symbol_t symbol::type() const
  {
    return type_;
  }

  ///
  /// \return the opcode of the symbol (an \c unsigned \c int used as primary
  /// key).
  ///
  inline
  opcode_t symbol::opcode() const
  {
    return opcode_;
  }

  ///
  /// \return \c true if the \a symbol has been automatically defined (e.g.
  ///         ADF / ADT), \c false otherwise (this is the default value).
  ///
  inline
  bool symbol::auto_defined() const
  {
    return false;
  }
}  // namespace vita

#endif  // SYMBOL_H
