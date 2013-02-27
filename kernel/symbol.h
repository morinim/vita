/**
 *
 *  \file symbol.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SYMBOL_H)
#define      SYMBOL_H

#include <string>

#include "vita.h"

namespace vita
{
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
    typedef std::shared_ptr<symbol> ptr;

    symbol(const std::string &, category_t, unsigned);

    opcode_t opcode() const;
    category_t category() const;

    bool terminal() const;
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
    virtual size_t arity() const = 0;

    /// Calculates the value of / performs the action associated to the symbol
    /// (it is implementation specific).
    virtual any eval(interpreter *) const = 0;

    virtual bool debug() const;

  public:  // Public data members.
    /// Weights is used by the symbol_set::roulette method to control the
    /// probability of extraction of the symbol.
    unsigned weight;

  private:  // Private data members.
    static opcode_t opc_count_;

    opcode_t     opcode_;

    category_t category_;

    std::string display_;
  };

  ///
  /// \param[in] dis string used for printing.
  /// \param[in] c category of the symbol.
  /// \param[in] w weight (used for random selection).
  ///
  inline
  symbol::symbol(const std::string &dis, category_t c, unsigned w)
    : weight(w), opcode_(opc_count_++), category_(c), display_(dis)
  {
    assert(debug());
  }

  ///
  /// \return \c 0.
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
  /// \return the category of the \a symbol.
  ///
  /// In strongly typed GP, every terminal has a type (i.e. category) and every
  /// function has types for each of its arguments and a type for its return
  /// value.
  ///
  inline
  category_t symbol::category() const
  {
    return category_;
  }

  ///
  /// \return the opcode of the symbol (used as primary key).
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
