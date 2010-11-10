/**
 *
 *  \file symbol.h
 *
 *  \author Manlio Morini
 *  \date 2009/10/14
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(SYMBOL_H)
#define      SYMBOL_H

#include <sstream>
#include <string>

#include <boost/any.hpp>

#include "vita.h"

namespace vita
{

  const symbol_t sym_void(0);
  const symbol_t sym_bool(1);
  const symbol_t sym_real(2);
  const symbol_t free_symbol(3);

  class interpreter;

  class symbol
  {
  public:
    symbol(const std::string &, symbol_t, unsigned);

    opcode_t opcode() const;
    bool terminal() const;

    symbol_t type() const;
    virtual symbol_t arg_type(unsigned) const = 0;

    virtual std::string display() const;
    virtual std::string display(int) const;
    virtual int init() const;

    virtual bool associative() const = 0;
    virtual bool parametric() const = 0;    

    virtual unsigned argc() const = 0;

    virtual boost::any eval(interpreter &) const = 0;

    bool check() const;

    unsigned weight;

  private:
    static unsigned _opc_count;

    const opcode_t     _opcode;

    const symbol_t       _type;

    const std::string _display;
  };

  /**
   * symbol
   * \param dis[in]
   * \param w[in]
   */
  inline
  symbol::symbol(const std::string &dis, symbol_t t, unsigned w)
    : weight(w), _opcode(++_opc_count), _type(t), _display(dis)
  {
    assert(check());
  }

  /**
   * init
   * \return
   */
  inline
  int
  symbol::init() const
  {
    return 0;
  }

  /**
   * terminal
   * \return true if this symbol is a terminal.
   */
  inline
  bool
  symbol::terminal() const
  {
    return !argc();
  }

  /**
   * type
   * \return the type of the symbol.
   */
  inline
  symbol_t
  symbol::type() const
  {
    return _type;
  }

  /**
   * opcode
   * \return the opcode of the symbol (an unsigned int used as primary key).
   */
  inline
  opcode_t
  symbol::opcode() const
  {
    return _opcode;
  }

  /**
   * display
   * \return the name of the symbol.
   */
  inline
  std::string
  symbol::display() const
  {
    return _display;
  }
    
}  // namespace vita

#endif  // SYMBOL_H
