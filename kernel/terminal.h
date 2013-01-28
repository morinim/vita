/**
 *
 *  \file terminal.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(TERMINAL_H)
#define      TERMINAL_H

#include "symbol.h"

namespace vita
{
  ///
  /// A terminal (leaf) node in the parse trees representing the programs in the
  /// population. A terminal might be a variable (input to the program), a
  /// constant value or a function taking no arguments (e.g. move-north).
  /// Terminals are symbols with an arity of zero.
  ///
  class terminal : public symbol
  {
  public:
    terminal(const std::string &, category_t, bool = false, bool = false,
             unsigned = default_weight);

    ///
    /// \return \c false (terminals haven't arguments and cannot be
    ///         associative).
    ///
    virtual bool associative() const { return false; }

    ///
    /// \return \c true if the terminal is an input variable.
    ///
    /// An input variable is a feature from the learning domain.
    ///
    bool input() const { return input_; }

    ///
    /// \return \c true if the terminal is parametric.
    ///
    virtual bool parametric() const { return parametric_; }

    ///
    /// \return 0 (0 arguments <=> terminal).
    ///
    virtual size_t arity() const { return 0; }

    virtual bool check() const;

  public:   // Public data members.
    static unsigned default_weight;

  private:  // Private data members.
    bool parametric_;
    bool      input_;
  };
}  // namespace vita

#endif  // TERMINAL_H
