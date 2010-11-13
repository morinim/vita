/**
 *
 *  \file symbol_set.cc
 *
 *  \author Manlio Morini
 *  \date 2010/11/13
 *
 *  This file is part of VITA
 *
 */

#include "symbol_set.h"
#include "adf.h"
#include "argument.h"
#include "random.h"

namespace vita
{

  /**
   * symbol_set
   */
  symbol_set::symbol_set()
  {
    clear();

    assert(check());
  }

  /**
   * clear
   */
  void
  symbol_set::clear()
  {
    _symbols.clear();
    _terminals.clear();
    _adf.clear();

    _arguments.clear();
    for (unsigned i(0); i < gene_args; ++i)
      _arguments.push_back(new argument(i));

    _sum = 0;
  }

  /**
   * arg
   * \param n[in]
   * \return
   */
  const argument *
  symbol_set::arg(unsigned n) const
  {
    assert(n < gene_args);
    return _arguments[n];
  }

  /**
   * insert
   * \param i[in]
   */
  void
  symbol_set::insert(symbol *const i)
  {
    assert(i && i->weight && i->check());
    
    _symbols.push_back(i);
    _sum += i->weight;

    terminal *const trml = dynamic_cast<terminal *>(i);
    if (trml)
    {
      assert(!i->argc());
      _terminals.push_back(trml);
    }
    else
    {
      adf *const df = dynamic_cast<adf *>(i);
      if (df)
        _adf.push_back(df);
    }

    assert(check());
  }

  /**
   * reset_adf_weights
   */
  void
  symbol_set::reset_adf_weights()
  {
    for (unsigned i(0); i < _adf.size(); ++i)
    {
      const unsigned delta(_adf[i]->weight/2);
      _sum -= delta;
      _adf[i]->weight -= delta; 
    }
  }

  /**
   * roulette
   * \param only_t[in]
   * \return a random terminal if only_t==true, else a random symbol.
   */
  const symbol *
  symbol_set::roulette(bool only_t) const
  {
    assert(_sum);

    if (only_t)
    {
      const unsigned i(random::between<unsigned>(0,_terminals.size()));

      assert( !dynamic_cast<argument *>(_terminals[i]) );
      return _terminals[i];
    }

    const unsigned slot(random::between<unsigned>(0,_sum)); 

    unsigned i(0);
    for (unsigned wedge(_symbols[i]->weight);
	 wedge <= slot && i+1 < _symbols.size();
	 wedge += _symbols[++i]->weight)
      ;

    assert( !dynamic_cast<argument *>(_symbols[i]) );
    return _symbols[i];
  }

  /**
   * decode
   * \param opcode[in]
   * \return a pointer to the class identified by 'opcode' (0 if not found).
   */
  const symbol *
  symbol_set::decode(unsigned opcode) const
  {
    for (unsigned i(0); i < _symbols.size(); ++i)
      if (_symbols[i]->opcode() == opcode)
	return _symbols[i];

    return 0;
  }

  /**
   * decode
   * \param dex[in]
   * \return a pointer to the class identified by 'dex' (0 if not found).
   */
  const symbol *
  symbol_set::decode(const std::string &dex) const
  {
    for (unsigned i(0); i < _symbols.size(); ++i)
      if (_symbols[i]->display() == dex)
	return _symbols[i];

    return 0;
  }

  /**
   * check
   * \return true if the individual passes the internal consistency check.
   */
  bool
  symbol_set::check() const
  {
    unsigned long sum(0);

    for (unsigned j(0); j < _symbols.size(); ++j)
    {
      if (!_symbols[j]->check())
	return false;

      sum += _symbols[j]->weight;

      bool found(false);
      if (dynamic_cast<terminal *>(_symbols[j]))
        for (unsigned i(0); i < _terminals.size() && !found; ++i)      
          found = (_symbols[j] ==_terminals[i]);
      else if (dynamic_cast<adf *>(_symbols[j]))
        for (unsigned i(0); i < _adf.size() && !found; ++i)      
          found = (_symbols[j] ==_adf[i]);
      else
        found = true;
     
      if (!found)
	return false;
    }
    
    if (sum != _sum)
      return false;

    // There should be one terminal at least.
    return _symbols.size()==0 || _terminals.size()>0;
  }

}  // Namespace vita
