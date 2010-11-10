/**
 *
 *  \file terminal.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */

#include "terminal.h"

namespace vita
{

  unsigned terminal::default_weight(100);

  /**
   * terminal
   * \param dis[in]
   * \param t[in]
   * \param w[in]
   * \param par[in]
   */
  terminal::terminal(const std::string &dis, symbol_t t, unsigned w, bool par)
    : symbol(dis,t,w), _parametric(par)
  {
    assert(check());
  }

  /**
   * check
   */
  bool
  terminal::check() const
  {
    return symbol::check();
  }

}  // Namespace vita
