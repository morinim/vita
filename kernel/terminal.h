/**
 *
 *  \file terminal.h
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */

#if !defined(TERMINAL_H)
#define      TERMINAL_H

#include "vita.h"
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
    terminal(const std::string &, symbol_t, bool=false, bool=false,
             unsigned=default_weight);

    symbol_t arg_type(unsigned) const;

    bool associative() const;
    bool input() const;
    bool parametric() const;

    unsigned argc() const;

    bool check() const;

    static unsigned default_weight;

  private:
    const bool _parametric;
    const bool      _input;
  };

  ///
  /// \return \a sym_void.
  ///
  /// No arguments for terminals!
  ///
  inline
  symbol_t
  terminal::arg_type(unsigned) const
  {
    return sym_void;
  }

  ///
  /// \return 0
  /// 0 arguments <=> terminal.
  ///
  inline
  unsigned
  terminal::argc() const
  {
    return 0;
  }

  ///
  /// \return false
  ///
  inline
  bool
  terminal::associative() const
  {
    return false;
  }

  ///
  /// \return true if the terminal is an input variable.
  ///
  inline
  bool
  terminal::input() const
  {
    return _input;
  }

  ///
  /// \return true if the terminal is parametric.
  ///
  inline
  bool
  terminal::parametric() const
  {
    return _parametric;
  }
    
}  // namespace vita

#endif  // TERMINAL_H
