/**
 *
 *  \file gene.cc
 *
 *  \author Manlio Morini
 *  \date 2011/05/11
 *
 *  This file is part of VITA
 *
 */

#include "gene.h"

#include "random.h"
#include "symbol.h"

namespace vita
{

  ///
  /// \param[in] sset the set of symbols to choose from.
  ///
  gene::gene(const symbol_set &sset)
  {
    sym = sset.roulette(true);
    
    if (sym->parametric())
      par = sym->init();
  }

  ///
  /// \param[in] sset the set of symbols to choose from.
  /// \param[in] from minimum index for function arguments.
  /// \param[in] sup upper bound for the indexes of function arguments.
  ///
  gene::gene(const symbol_set &sset, unsigned from, unsigned sup)
  {
    assert(from < sup);
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

  ///
  /// \param[in] sset the set of symbols to choose from.
  /// \param[in] var index of a special symbol in the \a symbol_set.
  ///
  gene::gene(const symbol_set &sset, unsigned var)
  {
    sym = sset.special(var);
    if (sym->parametric())
      par = sym->init();
  }

  ///
  /// \param[in] g second term of comparison.
  /// \return true if \c this == \a g
  ///
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
