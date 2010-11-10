/**
 *
 *  \file function.h
 *
 *  \author Manlio Morini
 *  \date 2009/10/20
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(FUNCTION_H)
#define      FUNCTION_H

#include <vector>

#include "vita.h"
#include "symbol.h"

namespace vita
{

  class function : public symbol
  {
  public:
    function(const std::string &, symbol_t, const std::vector<symbol_t> &, 
             unsigned, bool=false);
    function(const std::string &, symbol_t, unsigned, unsigned=default_weight,
             bool=false);

    bool associative() const;
    bool parametric() const;

    symbol_t arg_type(unsigned) const;
    unsigned argc() const;

    bool check() const;

    static unsigned default_weight;

  private:
    symbol_t _argt[gene_args];
    unsigned            _argc;
    const bool   _associative;
  };

  /**
   * argc
   * \return the number of arguments (0 arguments => terminal).
   */
  inline
  unsigned
  function::argc() const
  {
    assert(_argc);
    return _argc;
  }

  /**
   * arg_type
   * \param i[in]
   * \return i-th argument type of the function.
   */
  inline
  symbol_t
  function::arg_type(unsigned i) const
  {
    assert(i < gene_args);
    return _argt[i];
  }

  /**
   * associative
   * \return
   */
  inline
  bool
  function::associative() const
  {
    return _associative;
  }

  /**
   * parametric
   * \return false (function are never parametric). 
   */
  inline
  bool
  function::parametric() const
  {
    return false;
  }
    
}  // namespace vita

#endif  // FUNCTION_H
