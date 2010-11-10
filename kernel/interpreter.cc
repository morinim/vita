/**
 *
 *  \file interpreter.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */

#include "interpreter.h"
#include "adf.h"
#include "individual.h"
#include "symbol.h"

namespace vita
{

  /**
   * interpreter
   * \param ind[in]
   * \param ctx[in]
   * \param ip[in]
   */
  interpreter::interpreter(const individual &ind, 
                           interpreter *const ctx,
                           unsigned ip) 
    : _ip(ip), _context(ctx), _ind(ind), 
      _context_cache(ctx ? ctx->_ind.size() : 0)
  {
  }

  /**
   * run
   * \return
   */
  boost::any
  interpreter::run()
  {
    _ip = _ind._best;
    return _ind._code[_ip].sym->eval(*this);
  }

  /**
   * eval
   * \return
   */
  boost::any
  interpreter::eval()
  {
    const gene &g(_ind._code[_ip]);

    assert(g.sym->parametric());
    return g.par;
  }

  /**
   * eval
   * \param i[in]
   * \return
   */
  boost::any
  interpreter::eval(unsigned i)
  {
    const gene &g(_ind._code[_ip]);

    assert(i < g.sym->argc());

    const unsigned backup(_ip); 
    _ip = g.args[i];
    assert (_ip > backup);
    const boost::any ret(_ind._code[_ip].sym->eval(*this));
    _ip = backup;
    return ret;
    //return _ind._code[g.args[i]].sym->eval(interpreter(_ind,_context,
    //                                                   g.args[i]));
  }

  /**
   * eval_adf_arg
   * \param i[in]
   * \return
   */
  boost::any
  interpreter::eval_adf_arg(unsigned i)
  {
    const gene context_g(_context->_ind._code[_context->_ip]);

    assert( _context && _context->check() && i < gene_args && 
            dynamic_cast<const adf *>(context_g.sym) );

    if (_context_cache[context_g.args[i]].empty())
      _context_cache[context_g.args[i]] = _context->eval(i);

    return _context_cache[context_g.args[i]];
  }

  /**
   * check
   * \return true if the object passes the internal consistency check.
   */
  bool
  interpreter::check() const
  {
    return
      _ip < _ind._code.size() &&
      (!_context || _context->check());
  }

}  // Namespace vita
