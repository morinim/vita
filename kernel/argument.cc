/**
 *
 *  \file argument.cc
 *
 *  \author Manlio Morini
 *  \date 2010/11/13
 *
 *  This file is part of VITA
 *
 */

#include "argument.h"
#include "gene.h"
#include "interpreter.h"

namespace vita
{
  /**
   * argument
   * \param n[in]
   * \param t[in]
   */
  argument::argument(unsigned n) 
    : terminal("ARG",sym_void,0), _index(n)
  {
    assert(check());
  }

  /**
   * eval
   * \param agent[in]
   * \return
   */
  boost::any
  argument::eval(interpreter &agent) const
  {
    return agent.eval_adf_arg(_index);
  }

  /**
   * check
   * \return
   */
  bool
  argument::check() const
  {
    return _index < gene_args && terminal::check();
  }

}  // Namespace vita
