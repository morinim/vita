/**
 *
 *  \file function.cc
 *
 *  \author Manlio Morini
 *  \date 2010/11/13
 *
 *  This file is part of VITA
 *
 */

#include "function.h"

namespace vita
{

  unsigned function::default_weight(100);

  /**
   * function
   * \param dis[in]
   * \param t[in]
   * \param args[in]
   * \param w[in]
   * \param asve[in]
   */
  function::function(const std::string &dis, symbol_t t, 
                     const std::vector<symbol_t> &args, unsigned w, bool asve)
    : symbol(dis,t,w), _associative(asve)
  {
    assert(args.size() <= gene_args);

    for (_argc=0; _argc < args.size(); ++_argc)
      _argt[_argc] = args[_argc];

    for (unsigned i(argc()); i < gene_args; ++i)
      _argt[i] = sym_void;

    assert(check());
  }

  /**
   * function
   * \param dis[in]
   * \param t[in]
   * \param n[in]
   * \param w[in]
   * \param asve[in]
   */
  function::function(const std::string &dis, symbol_t t, 
                     unsigned n, unsigned w, bool asve)
    : symbol(dis,t,w), _argc(n), _associative(asve)
  {
    assert(n <= gene_args);

    unsigned i(0);
    for (; i <     _argc; ++i)   _argt[i] =        t;
    for (; i < gene_args; ++i)   _argt[i] = sym_void;

    assert(check());
  }

  /**
   * check
   * \return true if the individual passes the internal consistency check.
   */
  bool
  function::check() const
  {
    return _argc && _argc <= gene_args && symbol::check();
  }

}  // Namespace vita
