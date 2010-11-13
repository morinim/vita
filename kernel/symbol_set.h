/**
 *
 *  \file symbol_set.h
 *
 *  \author Manlio Morini
 *  \date 2010/11/13
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(SYMBOL_SET_H)
#define      SYMBOL_SET_H

#include <string>
#include <vector>

#include "vita.h"

namespace vita
{

  class adf;
  class argument;
  class symbol;
  class terminal;

  class symbol_set
  {
  public:
    symbol_set();

    void insert(symbol *const);

    const symbol *roulette(bool = false) const;
    const argument *arg(unsigned) const;

    void reset_adf_weights();

    const symbol *decode(unsigned) const;
    const symbol *decode(const std::string &) const;

    bool check() const;    

  private:
    void clear();

    std::vector<symbol *>     _symbols;
    std::vector<terminal *> _terminals;
    std::vector<adf *>            _adf;
    std::vector<argument *> _arguments;

    unsigned long _sum;
  };
  
}  // namespace vita

#endif  // SYMBOL_SET_H
