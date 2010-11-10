/**
 *
 *  \file gene.cc
 *
 *  \author Manlio Morini
 *  \date 2009/12/31
 *
 *  This file is part of VITA
 *
 */

#include "gene.h"

#include "random.h"
#include "symbol.h"

namespace vita
{

  /**
   * gene
   * \param sset[in]
   */
  gene::gene(const symbol_set &sset)
  {
    sym = sset.roulette(true);
    
    if (sym->parametric())
      par = sym->init();
  }

  /**
   * gene
   * \param sset[in]
   * \param from[in]
   * \param sup[in]
   */
  gene::gene(const symbol_set &sset, unsigned from, unsigned sup)
  {
    sym = sset.roulette(from==sup);
    
    if (sym->parametric())
      par = sym->init();
    else
    {
      const unsigned argc(sym->argc());
      for (unsigned j(0); j < argc; ++j)
        args[j] = random::between<unsigned>(from,sup);
    }
  }

  /**
   * operator==
   * \param g
   */
  bool
  gene::operator==(const gene &g) const
  { 
    if (sym != g.sym)
      return false;

    if (sym->parametric())
      return par==g.par;

    const unsigned n(sym->argc());
    for (unsigned i(0); i < n; ++i)
      if (args[i] != g.args[i])
        return false;

    return true;
  }

}  // namespace vita
