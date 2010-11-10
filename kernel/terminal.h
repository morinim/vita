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

  class terminal : public symbol
  {
  public:
    terminal(const std::string &, symbol_t, unsigned=default_weight, 
             bool=false);

    symbol_t arg_type(unsigned) const;

    bool associative() const;
    bool parametric() const;    

    unsigned argc() const;

    bool check() const;

    static unsigned default_weight;

  private:
    const bool _parametric;
  };

  /**
   * arg_type
   * \return 0
   * no arguments for terminals!
   */
  inline
  symbol_t
  terminal::arg_type(unsigned) const
  {
    return sym_void;
  }

  /**
   * argc
   * \return 0
   * 0 arguments <=> terminal.
   */
  inline
  unsigned
  terminal::argc() const
  {
    return 0;
  }

  /**
   * associative.
   * \return false
   */
  inline
  bool
  terminal::associative() const
  {
    return false;
  }

  /**
   * parametric
   * \return
   */
  inline
  bool
  terminal::parametric() const
  {
    return _parametric;
  }
    
}  // namespace vita

#endif  // TERMINAL_H
